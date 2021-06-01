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
  app1.SetPrefix("/prefix/data/download/15");
  app1.SetAttribute("NodeName", StringValue("15"));
  app1.SetAttribute("Query", StringValue("data8/initRecord0/initRecord1/initRecord1/trash/"));
  app1.Install(Node15);
  ndnGlobalRoutingHelper.AddOrigins("/prefix/data/download/15", Node15);
  
  

  Kademlia Knode15("Node15", "data15", 15);
  Kademlia *P_15 = &Knode15;
  Kademlia Knode6("node6", "data6", 6);
  Kademlia *P_6 = &Knode6;
  Kademlia Knode8("node8", "data8", 8);
  Kademlia *P_8 = &Knode8;
  Kademlia Knode9("node9", "data9", 9);
  Kademlia *P_9 = &Knode9;


  //node6.Node_info();
  set_data_store("Node15", "/prefix/data/store/15", P_15);
  set_data_management("Node15", "/prefix/data/query/15", P_15);
  P_15->Set_KBucket(P_6);
  set_data_store("Node6", "/prefix/data/store/6", P_6);
  set_data_management("Node6", "/prefix/data/query/6", P_6);
  P_6->Set_KBucket(P_8);
  set_data_store("Node8", "/prefix/data/store/8", P_8);
  set_data_management("Node8", "/prefix/data/query/8", P_8);
  P_8->Set_KBucket(P_9);
  set_data_store("Node9", "/prefix/data/store/9", P_9);
  set_data_management("Node9", "/prefix/data/query/9", P_9);


  Ptr<Node> Node11 = Names::Find<Node>("Node11");
  ndn::AppHelper app2("CustomerApp");
  app2.SetPrefix("/prefix/data/download/11");
  app2.SetAttribute("NodeName", StringValue("11"));
  app2.SetAttribute("Query", StringValue("data11/initRecord0/initRecord1/initRecord1/trash/"));
  app2.Install(Node11);
  ndnGlobalRoutingHelper.AddOrigins("/prefix/data/download/11", Node11);

  Kademlia Knode11("Node11", "data11", 11);
  Kademlia *P_11 = &Knode11;
  Kademlia Knode7("node7", "data7", 7);
  Kademlia *P_7 = &Knode7;
  Kademlia Knode4("node4", "data4", 4);
  Kademlia *P_4 = &Knode4;
  Kademlia Knode1("node1", "data1", 1);
  Kademlia *P_1 = &Knode1;

  set_data_store("Node11", "/prefix/data/store/11", P_11);
  set_data_management("Node11", "/prefix/data/query/11", P_11);
  P_11->Set_KBucket(P_7);
  set_data_store("Node7", "/prefix/data/store/7", P_7);
  set_data_management("Node7", "/prefix/data/query/7", P_7);
  P_7->Set_KBucket(P_4);
  set_data_store("Node4", "/prefix/data/store/4", P_4);
  set_data_management("Node4", "/prefix/data/query/4", P_4);
  P_4->Set_KBucket(P_1);
  set_data_store("Node1", "/prefix/data/store/1", P_1);
  set_data_management("Node1", "/prefix/data/query/1", P_1);


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
