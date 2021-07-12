/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2011-2015  Regents of the University of California.
 *
 * This file is part of ndnSIM. See AUTHORS for complete list of ndnSIM authors and
 * contributors.
 *
 * ndnSIM is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * ndnSIM is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * ndnSIM, e.g., in COPYING.md file.  If not, see <http://www.gnu.org/licenses/>.
 **/

// ndn-grid-topo-plugin.cpp

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/ndnSIM-module.h"

// for LinkStatusControl::FailLinks and LinkStatusControl::UpLinks
#include "ns3/ndnSIM/helper/ndn-link-control-helper.hpp"


#include "ns3/ndnSIM/apps/nccu_customer-app.hpp"
#include "ns3/ndnSIM/apps/nccu_producer-app.hpp"
#include "apps/kademlia.hpp"

#include "ns3/ndnSIM/apps/shop_service.hpp"
#include "ns3/ndnSIM/apps/data_management.hpp"
#include "ns3/ndnSIM/apps/data_store.hpp"

#include <random>
#include <set>


namespace ns3 {

/**
 * This scenario simulates a grid topology (using topology reader module)
 *
 * (consumer) -- ( ) ----- ( )
 *     |          |         |
 *    ( ) ------ ( ) ----- ( )
 *     |          |         |
 *    ( ) ------ ( ) -- (producer)
 *
 * All links are 1Mbps with propagation 10ms delay.
 *
 * FIB is populated using NdnGlobalRoutingHelper.
 *
 * Consumer requests data from producer with frequency 10 interests per second
 * (interests contain constantly increasing sequence number).
 *
 * For every received interest, producer replies with a data packet, containing
 * 1024 bytes of virtual payload.
 *
 * To run scenario and see what is happening, use the following command:
 *
 *     NS_LOG=ndn.Consumer:ndn.Producer ./waf --run=nccu_topology_ex
 */

std::string toBinary(int n)
{
    string r;
    while (n != 0){
        r += ( n % 2 == 0 ? "0" : "1" );
        n /= 2;
    }
    reverse(r.begin(), r.end());
    bitset<8> bs1(r);
    return bs1.to_string();
}


void set_data_store(std::string nodeName, std::string prefix, Kademlia* k_ptr )
{
  ndn::GlobalRoutingHelper ndnGlobalRoutingHelper;

  std::ostringstream address;
  address << k_ptr;
  std::string location = address.str();

  Ptr<Node> TargetNode = Names::Find<Node>(nodeName);
  ndn::AppHelper app("DataStore");
  app.SetPrefix(prefix);
  app.SetAttribute("Kademlia", StringValue(location));
  app.Install(TargetNode);
  ndnGlobalRoutingHelper.AddOrigins(prefix, TargetNode);
}

void set_data_management(std::string nodeName, std::string prefix, Kademlia* k_ptr )
{
  ndn::GlobalRoutingHelper ndnGlobalRoutingHelper;

  std::ostringstream address;
  address << k_ptr;
  std::string location = address.str();

  Ptr<Node> TargetNode = Names::Find<Node>(nodeName);
  ndn::AppHelper app("DataManage");
  app.SetPrefix(prefix);
  app.SetAttribute("Kademlia", StringValue(location));
  app.Install(TargetNode);
  ndnGlobalRoutingHelper.AddOrigins(prefix, TargetNode);
}

void set_customerApp(int targetNum, std::string query, Kademlia* kptr, int nodeNum, std::string shopList)
{
  std::default_random_engine generator(time(NULL));
  std::poisson_distribution<int> poisson(10);

  ndn::GlobalRoutingHelper ndnGlobalRoutingHelper;
  Order* Optr_head = new Order("init", 0, targetNum);
  int head = 0;
  int tail = 0;

  //加入shopList
  if (shopList.length() != 0)
  {
    Optr_head->setShopList(shopList);
  }
  
  for (int i = 0; i < targetNum; i++)
  {
    double timeStamp = poisson(generator);
    std::cout << timeStamp << std::endl;
    head = tail;
    tail = query.find_first_of("/", head);
    Optr_head->AddOrder(query.substr(head, tail-head), timeStamp, targetNum+1);
    tail++;
  }

  std::ostringstream address;
  address << kptr;
  std::string location = address.str();

  std::ostringstream address2;
  address2 << Optr_head;
  std::string queryString = address2.str();


  Ptr<Node> Node0 = Names::Find<Node>("Node" + to_string(nodeNum) );
  ndn::AppHelper app1("CustomerApp");
  app1.SetPrefix("/prefix/data/download/" + toBinary(nodeNum));
  app1.SetAttribute("NodeName", StringValue(toBinary(nodeNum)));
  app1.SetAttribute("Kademlia", StringValue(location));
  app1.SetAttribute("Query", StringValue(queryString));
  app1.Install(Node0);
  ndnGlobalRoutingHelper.AddOrigins("/prefix/data/download/" + toBinary(nodeNum), Node0);
  
}

int
main(int argc, char* argv[])
{
  CommandLine cmd;
  cmd.Parse(argc, argv);

  AnnotatedTopologyReader topologyReader("", 25);
  topologyReader.SetFileName("src/ndnSIM/examples/topologies/nccu_topo.txt");
  topologyReader.Read();

  // Install NDN stack on all nodes
  // 可以設定cs size,cache policy等
  ndn::StackHelper ndnHelper;
  ndnHelper.setCsSize(1);
  ndnHelper.InstallAll();

  // Set BestRoute strategy
  ndn::StrategyChoiceHelper::InstallAll("/", "/localhost/nfd/strategy/ncc");

  // Installing global routing interface on all nodes
  ndn::GlobalRoutingHelper ndnGlobalRoutingHelper;
  ndnGlobalRoutingHelper.InstallAll();


  Kademlia Knode0("Node0", "Node0", toBinary(0));
  Kademlia *P_0 = &Knode0;
  Kademlia Knode1("Node1", "Node1", toBinary(1));
  Kademlia *P_1 = &Knode1;
  Kademlia Knode2("Node2", "Node2", toBinary(2));
  Kademlia *P_2 = &Knode2;
  Kademlia Knode3("Node3", "Node3", toBinary(3));
  Kademlia *P_3 = &Knode3;
  Kademlia Knode4("Node4", "Node4", toBinary(4));
  Kademlia *P_4 = &Knode4;
  Kademlia Knode5("Node5", "Node5", toBinary(5));
  Kademlia *P_5 = &Knode5;
  Kademlia Knode6("Node6", "Node6", toBinary(6));
  Kademlia *P_6 = &Knode6;
  Kademlia Knode7("Node7", "Node7", toBinary(7));
  Kademlia *P_7 = &Knode7;
  Kademlia Knode8("Node8", "Node8", toBinary(8));
  Kademlia *P_8 = &Knode8;
  Kademlia Knode9("Node9", "Node9", toBinary(9));
  Kademlia *P_9 = &Knode9;
  Kademlia Knode10("Node10", "Node10", toBinary(10));
  Kademlia *P_10 = &Knode10;
  Kademlia Knode11("Node11", "Node11", toBinary(11));
  Kademlia *P_11 = &Knode11;
  Kademlia Knode12("Node12", "Node12", toBinary(12));
  Kademlia *P_12 = &Knode12;
  Kademlia Knode13("Node13", "Node13", toBinary(13));
  Kademlia *P_13 = &Knode13;
  Kademlia Knode14("Node14", "Node14", toBinary(14));
  Kademlia *P_14 = &Knode14;
  Kademlia Knode15("Node15", "Node15", toBinary(15));
  Kademlia *P_15 = &Knode15;
  Kademlia Knode16("Node16", "Node16", toBinary(16));
  Kademlia *P_16 = &Knode16;


  //node13.Node_info();
  set_data_store("Node0", "/prefix/data/store/" + toBinary(0), P_0);
  set_data_management("Node0", "/prefix/data/query/" + toBinary(0), P_0);
  P_0->Set_KBucket(P_1);
  P_0->Set_KBucket(P_2);
  P_0->Set_KBucket(P_3);
  P_0->Set_KBucket(P_4);
  P_0->Set_KBucket(P_5);
  P_0->Set_KBucket(P_8);
  P_0->Set_KBucket(P_11);
  P_0->Set_KBucket(P_14);

  set_data_store("Node1", "/prefix/data/store/" + toBinary(1), P_1);
  set_data_management("Node1", "/prefix/data/query/" + toBinary(1), P_1);
  P_1->Set_KBucket(P_5);
  P_1->Set_KBucket(P_2);
  P_1->Set_KBucket(P_3);
  P_1->Set_KBucket(P_0);
  P_1->Set_KBucket(P_4);
  P_1->Set_KBucket(P_6);
  P_1->Set_KBucket(P_7);
  P_1->Set_KBucket(P_8);
  P_1->Set_KBucket(P_11);

  set_data_store("Node2", "/prefix/data/store/" + toBinary(2), P_2);
  set_data_management("Node2", "/prefix/data/query/" + toBinary(2), P_2);
  P_2->Set_KBucket(P_1);
  P_2->Set_KBucket(P_0);
  P_2->Set_KBucket(P_4);
  P_2->Set_KBucket(P_8);
  P_2->Set_KBucket(P_3);
  P_2->Set_KBucket(P_5);
  P_2->Set_KBucket(P_9);
  P_2->Set_KBucket(P_10);
  P_2->Set_KBucket(P_14);

  set_data_store("Node3", "/prefix/data/store/" + toBinary(3), P_3);
  set_data_management("Node3", "/prefix/data/query/" + toBinary(3), P_3);
  P_3->Set_KBucket(P_1);
  P_3->Set_KBucket(P_0);
  P_3->Set_KBucket(P_4);
  P_3->Set_KBucket(P_11);
  P_3->Set_KBucket(P_5);
  P_3->Set_KBucket(P_2);
  P_3->Set_KBucket(P_14);
  P_3->Set_KBucket(P_12);
  P_3->Set_KBucket(P_13);

  set_data_store("Node4", "/prefix/data/store/" + toBinary(4), P_4);
  set_data_management("Node4", "/prefix/data/query/" + toBinary(4), P_4);
  P_4->Set_KBucket(P_0);
  P_4->Set_KBucket(P_2);
  P_4->Set_KBucket(P_3);
  P_4->Set_KBucket(P_14);
  P_4->Set_KBucket(P_8);
  P_4->Set_KBucket(P_1);
  P_4->Set_KBucket(P_11);
  P_4->Set_KBucket(P_15);
  P_4->Set_KBucket(P_16);
  

  set_data_store("Node5", "/prefix/data/store/" + toBinary(5), P_5);
  set_data_management("Node5", "/prefix/data/query/" + toBinary(5), P_5);
  P_5->Set_KBucket(P_0);
  P_5->Set_KBucket(P_1);
  P_5->Set_KBucket(P_2);
  P_5->Set_KBucket(P_3);
  P_5->Set_KBucket(P_6);
  P_5->Set_KBucket(P_7);

  set_data_store("Node6", "/prefix/data/store/" + toBinary(6), P_6);
  set_data_management("Node6", "/prefix/data/query/" + toBinary(6), P_6);
  P_6->Set_KBucket(P_1);
  P_6->Set_KBucket(P_5);
  P_6->Set_KBucket(P_7);
  
  set_data_store("Node7", "/prefix/data/store/" + toBinary(7), P_7);
  set_data_management("Node7", "/prefix/data/query/" + toBinary(7), P_7);
  P_7->Set_KBucket(P_1);
  P_7->Set_KBucket(P_5);
  P_7->Set_KBucket(P_6);

  set_data_store("Node8", "/prefix/data/store/" + toBinary(8), P_8);
  set_data_management("Node8", "/prefix/data/query/" + toBinary(8), P_8);
  P_8->Set_KBucket(P_1);
  P_8->Set_KBucket(P_0);
  P_8->Set_KBucket(P_4);
  P_8->Set_KBucket(P_2);
  P_8->Set_KBucket(P_9);
  P_8->Set_KBucket(P_10);

  set_data_store("Node9", "/prefix/data/store/" + toBinary(9), P_9);
  set_data_management("Node9", "/prefix/data/query/" + toBinary(9), P_9);
  P_9->Set_KBucket(P_2);
  P_9->Set_KBucket(P_8);
  P_9->Set_KBucket(P_10);
  
  set_data_store("Node10", "/prefix/data/store/" + toBinary(10), P_10);
  set_data_management("Node10", "/prefix/data/query/" + toBinary(10), P_10);
  P_10->Set_KBucket(P_2);
  P_10->Set_KBucket(P_8);
  P_10->Set_KBucket(P_9);

  set_data_store("Node11", "/prefix/data/store/" + toBinary(11), P_11);
  set_data_management("Node11", "/prefix/data/query/" + toBinary(11), P_11);
  P_11->Set_KBucket(P_0);
  P_11->Set_KBucket(P_1);
  P_11->Set_KBucket(P_4);
  P_11->Set_KBucket(P_3);
  P_11->Set_KBucket(P_12);
  P_11->Set_KBucket(P_13);
  
  set_data_store("Node12", "/prefix/data/store/" + toBinary(12), P_12);
  set_data_management("Node12", "/prefix/data/query/" + toBinary(12), P_12);
  P_12->Set_KBucket(P_3);
  P_12->Set_KBucket(P_11);
  P_12->Set_KBucket(P_13);

  set_data_store("Node13", "/prefix/data/store/" + toBinary(13), P_13);
  set_data_management("Node13", "/prefix/data/query/" + toBinary(13), P_13);
  P_13->Set_KBucket(P_3);
  P_13->Set_KBucket(P_11);
  P_13->Set_KBucket(P_12);

  set_data_store("Node14", "/prefix/data/store/" + toBinary(14), P_14);
  set_data_management("Node14", "/prefix/data/query/" + toBinary(14), P_14);
  P_14->Set_KBucket(P_0);
  P_14->Set_KBucket(P_2);
  P_14->Set_KBucket(P_3);
  P_14->Set_KBucket(P_4);
  P_14->Set_KBucket(P_15);
  P_14->Set_KBucket(P_16);

  set_data_store("Node15", "/prefix/data/store/" + toBinary(15), P_15);
  set_data_management("Node15", "/prefix/data/query/" + toBinary(15), P_15);
  P_15->Set_KBucket(P_4);
  P_15->Set_KBucket(P_14);
  P_15->Set_KBucket(P_16);


  set_data_store("Node16", "/prefix/data/store/" + toBinary(16), P_16);
  set_data_management("Node16", "/prefix/data/query/" + toBinary(16), P_16);
  P_16->Set_KBucket(P_4);
  P_16->Set_KBucket(P_14);
  P_16->Set_KBucket(P_15);


  int targetNum = 0;

  set_customerApp(5, "food/food/food/food/food/", P_0, 0, "00000001");
  set_customerApp(targetNum, "food/food/food/food/food/", P_1, 1, "");
  set_customerApp(5, "food/food/food/food/food/", P_2, 2, "00000000");
  set_customerApp(targetNum, "food/food/food/food/food/", P_3, 3, "");
  set_customerApp(targetNum, "food/food/food/food/food/", P_4, 4, "");

  set_customerApp(targetNum, "food/food/food/food/food/", P_5, 5, "");
  set_customerApp(targetNum, "food/food/food/food/food/", P_6, 6, "");
  set_customerApp(targetNum, "food/food/food/food/food/", P_7, 7, "");
  set_customerApp(targetNum, "food/food/food/food/food/", P_8, 8, "");
  set_customerApp(targetNum, "food/food/food/food/food/", P_9, 9, "");

  set_customerApp(targetNum, "food/food/food/food/food/", P_10, 10, "");
  set_customerApp(targetNum, "food/food/food/food/food/", P_11, 11, "");
  set_customerApp(targetNum, "food/food/food/food/food/", P_12, 12, "");
  set_customerApp(targetNum, "food/food/food/food/food/", P_13, 13, "");
  set_customerApp(targetNum, "food/food/food/food/food/", P_14, 14, "");
  set_customerApp(targetNum, "food/food/food/food/food/", P_15, 15, "");
  set_customerApp(targetNum, "food/food/food/food/food/", P_16, 16, "");
    


  // Calculate and install FIBs
  ndn::GlobalRoutingHelper::CalculateRoutes();

  Ptr<Node> Fail_node3 = Names::Find<Node>("Node3");
  Ptr<Node> Fail_node4 = Names::Find<Node>("Node4");
  Ptr<Node> Fail_node0 = Names::Find<Node>("Node0");
  Ptr<Node> Fail_node2 = Names::Find<Node>("Node2");
   //Simulator::Schedule(Seconds(10.0), ndn::LinkControlHelper::FailLink, Fail_node3, Fail_node4);
   //Simulator::Schedule(Seconds(3.0), ndn::LinkControlHelper::FailLink, Fail_node4, Fail_node0);
   //Simulator::Schedule(Seconds(20.0), ndn::LinkControlHelper::FailLink, Fail_node0, Fail_node2);


  Simulator::Stop(Seconds(20.0));

  Simulator::Run();
  Simulator::Destroy();

  return 0;
}

} // namespace ns3

int
main(int argc, char* argv[])
{
  return ns3::main(argc, argv);
}
