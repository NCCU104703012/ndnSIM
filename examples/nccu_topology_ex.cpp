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
  ndnHelper.setCsSize(0);
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
  app1.SetPrefix("/prefix/data/download/15");
  app1.SetAttribute("NodeName", StringValue("15"));
  app1.Install(Node15);
  //ndnGlobalRoutingHelper.AddOrigins("/prefix/data/store/6", Node15);
  
  

  Kademlia Knode15("Node15", "data15", 15);
  Kademlia *P_15 = &Knode15;
  Kademlia Knode6("node6", "data6", 6);
  Kademlia *P_6 = &Knode6;
  Kademlia Knode8("node8", "data8", 8);
  Kademlia *P_8 = &Knode8;


  //node6.Node_info();
  set_data_store("Node15", "/prefix/data/store/15", P_15);
  set_data_management("Node15", "/prefix/data/query/15", P_15);
  P_15->Set_KBucket(P_6);
  set_data_store("Node6", "/prefix/data/store/6", P_6);
  set_data_management("Node6", "/prefix/data/query/6", P_6);
  P_6->Set_KBucket(P_8);
  set_data_store("Node8", "/prefix/data/store/8", P_8);
  set_data_management("Node8", "/prefix/data/query/8", P_8);


  // Ptr<Node> Node8 = Names::Find<Node>("Node8");
  // ndn::AppHelper app2("ProudcerApp");
  // app2.Install(Node8);

  // consumer_set("Node7", "/prefix/food/one", "10");
  // consumer_set("Node15", "/prefix/clothes", "0");
  // consumer_set("Node13", "/prefix/clothes", "5");


  // producer_set("Node10", "/prefix/food", "1024");
  // producer_set("Node15", prefix_clothes, "1024");
  // producer_set("Node9", prefix_clothes, "1024");



  // Calculate and install FIBs
  ndn::GlobalRoutingHelper::CalculateRoutes();

  Ptr<Node> Fail_node3 = Names::Find<Node>("Node3");
  Ptr<Node> Fail_node4 = Names::Find<Node>("Node4");
  Ptr<Node> Fail_node0 = Names::Find<Node>("Node0");
  Ptr<Node> Fail_node2 = Names::Find<Node>("Node2");
   //Simulator::Schedule(Seconds(10.0), ndn::LinkControlHelper::FailLink, Fail_node3, Fail_node4);
   //Simulator::Schedule(Seconds(3.0), ndn::LinkControlHelper::FailLink, Fail_node4, Fail_node0);
   //Simulator::Schedule(Seconds(20.0), ndn::LinkControlHelper::FailLink, Fail_node0, Fail_node2);

  // Kademlia node0("node0", "0");
  // Kademlia node2("node2", "2");
  // Kademlia node8("node8", "8");
  // Kademlia node9("node9", "9");

  // Kademlia *P_9 = &node9;
  // Kademlia *P_0 = &node0;
  // Kademlia *P_8 = &node8;
  // Kademlia *P_2 = &node2;

  // node0.SetNext(P_2);
  // node2.SetNext(P_8);
  // node8.SetNext(P_9);
  // node9.SetNext(P_9);

  // cout << "***********start test" << endl;
  // Test_Kad(P_0, "9");
  // cout << "***********done test" << endl;


  ndn::FibHelper::AddRoute("Node8", "/prefix/data/query/15/8/data8", "Node2", 1);
  ndn::FibHelper::AddRoute("Node2", "/prefix/data/query/15/8/data8", "Node4", 1);
  ndn::FibHelper::AddRoute("Node4", "/prefix/data/query/15/8/data8", "Node14", 1);
  ndn::FibHelper::AddRoute("Node14", "/prefix/data/query/15/8/data8", "Node15", 1);
  


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
