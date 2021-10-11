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
#include "ns3/ndnSIM/apps/data_management_origin.hpp"

#include <random>
#include <set>
#include <time.h>

//kad演算法 DataManageOrigin ＆ DataManage
std::string Query_Algorithm = "DataManage";

//節點數量
// int NodeNumber = 17;
int NodeNumber = 45;

//一個節點產生的order數量
int OrderNumber = 50;

//平均幾秒產生一筆資料
int Record_Poisson = 3600;
//分母 化小數點用
int Record_Poisson_div = 1;

//一個節點顧客產生數量
int GuestNumber = 50;

//平均幾秒處理下一個order
int Guest_Poisson = 3600;
int Guest_Poisson_div = 1;

//初始K桶大小
int Kbuk_Number = 4;

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

// std::random_device rd;
// std::default_random_engine generator{rd()};
std::default_random_engine generator(time(NULL));

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

void set_data_management(std::string nodeName, std::string prefix, Kademlia* k_ptr, std::string o_ptr_string )
{
  ndn::GlobalRoutingHelper ndnGlobalRoutingHelper;

  std::ostringstream address;
  address << k_ptr;
  std::string location = address.str();

  Ptr<Node> TargetNode = Names::Find<Node>(nodeName);
  ndn::AppHelper app(Query_Algorithm);
  app.SetPrefix(prefix);
  app.SetAttribute("Kademlia", StringValue(location));
  app.SetAttribute("Query", StringValue(o_ptr_string));
  app.Install(TargetNode);
  ndnGlobalRoutingHelper.AddOrigins(prefix, TargetNode);
}

void set_customerApp(int targetNum, std::string query, Kademlia* kptr, int nodeNum, std::set<int> shopList)
{
  std::poisson_distribution<int> poisson(Guest_Poisson);
  std::poisson_distribution<int> poisson_record(Record_Poisson);
  ndn::GlobalRoutingHelper ndnGlobalRoutingHelper;
  Order* Optr_head = new Order("init", "init", 0, targetNum);
  int head = 0;
  int tail = 0;

  //加入shopList
  std::set<std::string> shopList_string;
  std::set<int>::iterator i;
  for (i = shopList.begin(); i != shopList.end(); ++i)
  {
    int node = *i;
    shopList_string.insert(toBinary(node));
  }
  
  Optr_head->setShopList(shopList_string);

  //產生Guest list，用來產生實際record
  int record_count = 0;
  double totalTime = (double)poisson_record(generator)/Record_Poisson_div;
  Guest* Gptr_head = new Guest("Guest_Record_Node" + to_string(nodeNum) + "_" + to_string(record_count), totalTime);
  Guest* Gptr_temp = Gptr_head;

  //產生資料
  for (int i = 0; i < GuestNumber; i++)
  {
    record_count++;
    totalTime = totalTime + (double)poisson_record(generator)/Record_Poisson_div;
    Guest* newEntry = new Guest("Guest_Record_Node" + to_string(nodeNum) + "_" + to_string(record_count), totalTime);
    Gptr_temp->setNext(newEntry);
    Gptr_temp = Gptr_temp->getNext();

  }
  

  
  //加入order
  totalTime = 0;
  for (int i = 0; i < targetNum; i++)
  {
    std::string orderName = "newRecord_Node" + to_string(nodeNum) + "_" + to_string(i);

    totalTime = totalTime + (double)poisson(generator)/Guest_Poisson_div;
    std::cout << totalTime << " ";
    head = tail;
    tail = query.find_first_of("/", head);
    Optr_head->AddOrder(orderName, query.substr(head, tail-head), totalTime, targetNum+1);
    tail++;


    //在每個Order間空出timeout時間，避免指令重疊
    totalTime++;
  }

  //將排序好的order加入流水號 以便query function 參考
  Order* tempPtr = Optr_head;
  int serial_num = 0;
  while (tempPtr != NULL)
  {
    tempPtr->setSerial_num(serial_num);
    serial_num++;
    tempPtr = tempPtr->getNext();
    //std::cout << "set serialnum: " << serial_num << "  " << tempPtr->getOrderName() << std::endl;
  }

  std::cout <<  std::endl;


  std::ostringstream address;
  address << kptr;
  std::string location = address.str();

  std::ostringstream address2;
  address2 << Optr_head;
  std::string queryString = address2.str();

  std::ostringstream address3;
  address3 << Gptr_head;
  std::string gptrString = address3.str();


  Ptr<Node> Node0 = Names::Find<Node>("Node" + to_string(nodeNum) );
  ndn::AppHelper app1("CustomerApp");
  app1.SetPrefix("/prefix/data/download/" + toBinary(nodeNum));
  app1.SetAttribute("NodeName", StringValue(toBinary(nodeNum)));
  app1.SetAttribute("Kademlia", StringValue(location));
  app1.SetAttribute("Query", StringValue(queryString));
  app1.SetAttribute("Guest", StringValue(gptrString));
  app1.SetAttribute("Query_Algorithm", StringValue(Query_Algorithm));
  app1.Install(Node0);
  ndnGlobalRoutingHelper.AddOrigins("/prefix/data/download/" + toBinary(nodeNum), Node0);
  
  //設定data management模組
  set_data_management("Node" + to_string(nodeNum), "/prefix/data/query/" + toBinary(nodeNum), kptr, queryString);
}

void generate_node(){

  //將節點指標存成陣列
  Kademlia *kptr_arr[NodeNumber];
  for (int i = 0; i < NodeNumber; i++)
  {
    std::string nodeName = "Node" + to_string(i);
    kptr_arr[i] = new Kademlia(nodeName, nodeName, toBinary(i));
    set_data_store(nodeName, "/prefix/data/store/" + toBinary(i), kptr_arr[i]);
  }

  //設定K桶，目前以名字相近的四個節點為K桶
  for (int i = 0; i < NodeNumber; i++)
  {
    int targetNode = i-2;
    for (int m = 0; m < Kbuk_Number+1 ; m++)
    {
      if (targetNode < 0 || targetNode == i || targetNode >= NodeNumber)
      {
        targetNode++;
      }
      else
      {
        kptr_arr[i]->Set_KBucket(kptr_arr[targetNode]->GetKId());
        std::cout<<"set Kbucket: " << kptr_arr[targetNode]->GetNodeName() << " into " << kptr_arr[i]->GetNodeName()<<std::endl;
        targetNode++;
      }
    }
  }

  //設定合作商家，暫時分為三群
  std::set<int> set1 = {0, 1, 2, 3, 4, 5};
  std::set<int> set2 = {6, 7, 8, 9, 10};
  std::set<int> set3 = {11, 12, 13, 14, 15, 16};
  
  for (int i = 0; i < NodeNumber; i++)
  {
    std::set<int> tempSet;
    if (set1.find(i) != set1.end()){tempSet = set1;}
    else if(set2.find(i) != set2.end()){tempSet = set2;}
    else{tempSet = set3;}

    tempSet.erase(i);

    set_customerApp(OrderNumber, "food/food/food/food/food/", kptr_arr[i], i, tempSet);
  }
  
}

int
main(int argc, char* argv[])
{
  CommandLine cmd;
  cmd.Parse(argc, argv);


  AnnotatedTopologyReader topologyReader("", 25);
  //topologyReader.SetFileName("src/ndnSIM/examples/topologies/nccu_topo.txt");
  topologyReader.SetFileName("/home/nccu108753108/ndnSIM/ns-3/nccu_visualization/nccu_topo50.txt");
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

  //產生資料結構，初始化節點模組
  generate_node();
    

  // Calculate and install FIBs
  ndn::GlobalRoutingHelper::CalculateRoutes();

  Ptr<Node> Fail_node3 = Names::Find<Node>("Node3");
  Ptr<Node> Fail_node4 = Names::Find<Node>("Node4");
  Ptr<Node> Fail_node0 = Names::Find<Node>("Node0");
  Ptr<Node> Fail_node2 = Names::Find<Node>("Node2");
   //Simulator::Schedule(Seconds(10.0), ndn::LinkControlHelper::FailLink, Fail_node3, Fail_node4);
   //Simulator::Schedule(Seconds(3.0), ndn::LinkControlHelper::FailLink, Fail_node4, Fail_node0);
  //  Simulator::Schedule(Seconds(1.0), ndn::LinkControlHelper::FailLink, Fail_node0, Fail_node2);
  //  Simulator::Schedule(Seconds(10.0), ndn::LinkControlHelper::UpLink, Fail_node0, Fail_node2);

  ndn::L3RateTracer::InstallAll("rate-trace.txt", Seconds(0.5));
  L2RateTracer::InstallAll("drop-trace.txt", Seconds(5));
  // ndn::CsTracer::InstallAll("cs-trace.txt", Seconds(1));

  Simulator::Stop(Seconds(10000000.0));

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
