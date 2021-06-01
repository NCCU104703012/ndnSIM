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

void producer_set(std::string node, std::string prefix, std::string payloadsize){
  Ptr<Node> producer = Names::Find<Node>(node);
  ndn::AppHelper producerHelper("ns3::ndn::ProducerApp");
  ndn::GlobalRoutingHelper ndnGlobalRoutingHelper;
  producerHelper.SetPrefix(prefix);
  producerHelper.SetAttribute("PayloadSize", StringValue(payloadsize));
  producerHelper.Install(producer);
  ndnGlobalRoutingHelper.AddOrigins(prefix, producer);
}

void consumer_set(std::string node, std::string prefix, std::string frequency){
  Ptr<Node> consumer = Names::Find<Node>(node);
  ndn::AppHelper consumerHelper("ns3::ndn::ConsumerCbr");
  //ndn::GlobalRoutingHelper ndnGlobalRoutingHelper;
  consumerHelper.SetPrefix(prefix);
  consumerHelper.SetAttribute("Frequency", StringValue(frequency)); // 100 interests a second
  consumerHelper.Install(consumer);
  //ndnGlobalRoutingHelper.AddOrigins(prefix, consumer);
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
  ndnHelper.setCsSize(5);
  ndnHelper.InstallAll();

  // Set BestRoute strategy
  ndn::StrategyChoiceHelper::InstallAll("/", "/localhost/nfd/strategy/ncc");

  // Installing global routing interface on all nodes
  ndn::GlobalRoutingHelper ndnGlobalRoutingHelper;
  ndnGlobalRoutingHelper.InstallAll();

  // // Getting containers for the consumer/producer
  // // 分為food & clothes兩類
  // std::string prefix_DataStore = "/prefix/data/store";
  // std::string prefix_clothes = "/prefix/clothes";


  Ptr<Node> Node15 = Names::Find<Node>("Node15");
  ndn::AppHelper app1("CustomerApp");
  app1.SetPrefix("/prefix/data/download/" + toBinary(15));
  app1.SetAttribute("NodeName", StringValue(toBinary(15)));
  app1.SetAttribute("TargetNode", StringValue(toBinary(0)));
  app1.SetAttribute("Query", StringValue("Record123/initRecord0/initRecord1/initRecord1/trash/"));
  app1.SetAttribute("Record", StringValue("Record123/initRecord0/initRecord1/initRecord1/trash/"));
  app1.Install(Node15);
  ndnGlobalRoutingHelper.AddOrigins("/prefix/data/download/" + toBinary(15), Node15);
  
  

  Kademlia Knode15("Node15", "Node15", toBinary(15));
  Kademlia *P_15 = &Knode15;
  Kademlia Knode13("node13", "Node13", toBinary(13));
  Kademlia *P_13 = &Knode13;
  Kademlia Knode10("node10", "Node10", toBinary(10));
  Kademlia *P_10 = &Knode10;
  Kademlia Knode8("node8", "Node8", toBinary(8));
  Kademlia *P_8 = &Knode8;


  //node13.Node_info();
  set_data_store("Node15", "/prefix/data/store/" + toBinary(15), P_15);
  set_data_management("Node15", "/prefix/data/query/" + toBinary(15), P_15);
  P_15->Set_KBucket(P_13);
  set_data_store("Node13", "/prefix/data/store/" + toBinary(13), P_13);
  set_data_management("Node13", "/prefix/data/query/" + toBinary(13), P_13);
  P_13->Set_KBucket(P_10);
  set_data_store("Node10", "/prefix/data/store/" + toBinary(10), P_10);
  set_data_management("Node10", "/prefix/data/query/" + toBinary(10), P_10);
  P_10->Set_KBucket(P_8);
  set_data_store("Node8", "/prefix/data/store/" + toBinary(8), P_8);
  set_data_management("Node8", "/prefix/data/query/" + toBinary(8), P_8);

//================================================
//設定第二條線
//================================================

  // Ptr<Node> Node11 = Names::Find<Node>("Node11");
  // ndn::AppHelper app2("CustomerApp");
  // app2.SetPrefix("/prefix/data/download/" + toBinary(11));
  // app2.SetAttribute("NodeName", StringValue(toBinary(11)));
  // app2.SetAttribute("TargetNode", StringValue(toBinary(0)));
  // app2.SetAttribute("Query", StringValue("Record456/11_initRecord0/11_initRecord1/11_initRecord1/11_trash/"));
  // app2.SetAttribute("Record", StringValue("Record456/11_initRecord0/11_initRecord1/11_initRecord1/11_trash/"));
  // app2.Install(Node11);
  // ndnGlobalRoutingHelper.AddOrigins("/prefix/data/download/" + toBinary(11), Node11);
  
  

  // Kademlia Knode11("Node11", "Node11", toBinary(11));
  // Kademlia *P_11 = &Knode11;
  // Kademlia Knode4("node4", "Node4", toBinary(4));
  // Kademlia *P_4 = &Knode4;
  // Kademlia Knode0("node0", "Node0", toBinary(0));
  // Kademlia *P_0 = &Knode0;


  // //node7.Node_info();
  // set_data_store("Node11", "/prefix/data/store/" + toBinary(11), P_11);
  // set_data_management("Node11", "/prefix/data/query/" + toBinary(11), P_11);
  // P_11->Set_KBucket(P_10);
  // set_data_store("Node4", "/prefix/data/store/" + toBinary(4), P_4);
  // set_data_management("Node4", "/prefix/data/query/" + toBinary(4), P_4);
  // P_4->Set_KBucket(P_0);
  // set_data_store("Node0", "/prefix/data/store/" + toBinary(0), P_0);
  // set_data_management("Node0", "/prefix/data/query/" + toBinary(0), P_0);



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
