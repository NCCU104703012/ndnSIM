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
  ndn::AppHelper producerHelper("ns3::ndn::Producer");
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
  ndnHelper.setCsSize(1000);
  ndnHelper.InstallAll();

  // Set BestRoute strategy
  ndn::StrategyChoiceHelper::InstallAll("/", "/localhost/nfd/strategy/ncc");

  // Installing global routing interface on all nodes
  ndn::GlobalRoutingHelper ndnGlobalRoutingHelper;
  ndnGlobalRoutingHelper.InstallAll();

  // // Getting containers for the consumer/producer
  // // 分為food & clothes兩類
  std::string prefix_food = "/prefix/food";
  std::string prefix_clothes = "/prefix/clothes";


  consumer_set("Node7", prefix_food, "10");
  consumer_set("Node12", prefix_clothes, "10");
  consumer_set("Node13", prefix_clothes, "5");


  producer_set("Node10", prefix_food, "1024");
  producer_set("Node15", prefix_clothes, "1024");
  producer_set("Node9", prefix_clothes, "1024");



  // Calculate and install FIBs
  ndn::GlobalRoutingHelper::CalculateRoutes();

  Ptr<Node> Fail_node3 = Names::Find<Node>("Node3");
  Ptr<Node> Fail_node4 = Names::Find<Node>("Node4");
  Ptr<Node> Fail_node0 = Names::Find<Node>("Node0");
   Simulator::Schedule(Seconds(10.0), ndn::LinkControlHelper::FailLink, Fail_node3, Fail_node4);
   Simulator::Schedule(Seconds(20.0), ndn::LinkControlHelper::FailLink, Fail_node4, Fail_node0);

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
