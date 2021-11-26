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

#include <stdio.h>
#include <sqlite3.h>

//kad演算法 DataManageOrigin ＆ DataManage
std::string Query_Algorithm = "DataManageOrigin";

//節點數量
// int NodeNumber = 17;
int NodeNumber = 100;

//一個節點產生的order數量
int OrderNumber = 5;

//order開始時間
int orderStartTime = 3600;

//平均幾秒處理下一個order
int Guest_Poisson = 3600;
int Guest_Poisson_div = 1;

//初始K桶大小
int Kbuk_Number = 5;

//是否設定初始K桶
bool set_kbucket_bool = true;

//商店合作鏈大小
int ShopChain_num = 5;

//程式結束時間
int StopTime = 30000;


static int callback(void *NotUsed, int argc, char **argv, char **azColName){
   int i;
   std::cout << "get one data ";
   for(i=0; i<argc; i++){
      //printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
      std::cout << argv[i] << "  ";
   }
   printf("\n");
   return 0;
}

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

std::string hashNodeName(std::string NodeName)
{
  std::size_t biTemp = std::hash<std::string>{}(NodeName);
  std::string binaryNodeName = std::bitset<8>(biTemp).to_string();
  return binaryNodeName;
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

  std::cout << "set manage at " << nodeName << " prefix: " << prefix << " KID: " <<  k_ptr->GetNodeName() <<"\n";

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
  //std::poisson_distribution<int> poisson_record(Record_Poisson);
  // std::uniform_int_distribution<int> distribution(0,Guest_Poisson);
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
    std::string nodeName = "Node" + to_string(node);
    shopList_string.insert(hashNodeName(nodeName));
  }
  
  Optr_head->setShopList(shopList_string);

  //產生Guest list，用來產生實際record
  double totalTime = (double)poisson(generator);
  Guest* Gptr_head = new Guest("Guest_Record_Node" + to_string(nodeNum), totalTime);
  

  //加入order
  totalTime = orderStartTime;
  for (int i = 0; i < targetNum; i++)
  {
    std::string orderName = "newRecord_Node" + to_string(nodeNum) + "_" + to_string(i);

    totalTime = totalTime + (double)poisson(generator)/Guest_Poisson_div;
    // std::cout << totalTime << " ";
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
  app1.SetPrefix("/prefix/data/download/" + kptr->GetKId());
  app1.SetAttribute("NodeName", StringValue(kptr->GetKId()));
  app1.SetAttribute("Kademlia", StringValue(location));
  app1.SetAttribute("Query", StringValue(queryString));
  app1.SetAttribute("Guest", StringValue(gptrString));
  app1.SetAttribute("Query_Algorithm", StringValue(Query_Algorithm));
  app1.Install(Node0);
  ndnGlobalRoutingHelper.AddOrigins("/prefix/data/download/" + kptr->GetKId(), Node0);
  
  //設定data management模組
  set_data_management("Node" + to_string(nodeNum), "/prefix/data/query/" + kptr->GetKId(), kptr, queryString);

  //設定data management模組，為了NDN fault tolerant
  if(Query_Algorithm == "DataManage"){
    // std::string prefix0 = kptr->GetKId().substr(0,8) + "xxxxxxxx";
    std::string prefix1 = kptr->GetKId().substr(0,4) + "xxxx";
    std::string prefix2 = kptr->GetKId().substr(0,6) + "xx";
    std::string prefix3 = kptr->GetKId().substr(0,7) + "x";
    std::cout << "KID: " << kptr->GetKId() << " P1: " << prefix1 << " P2: " << prefix2 << " P3: " << prefix3 << "\n";
    // set_data_management("Node" + to_string(nodeNum), "/prefix/data/query/" + prefix0, kptr, queryString);
    set_data_management("Node" + to_string(nodeNum), "/prefix/data/query/" + prefix1, kptr, queryString);
    set_data_management("Node" + to_string(nodeNum), "/prefix/data/query/" + prefix2, kptr, queryString);
    set_data_management("Node" + to_string(nodeNum), "/prefix/data/query/" + prefix3, kptr, queryString);
  }
}

void generate_node(sqlite3* db){

  //將節點指標存成陣列
  Kademlia *kptr_arr[NodeNumber];
  std::set<std::string> k_id_check_set;
  for (int i = 0; i < NodeNumber; i++)
  {
    std::string nodeName = "Node" + to_string(i);
    std::string k_id = hashNodeName(nodeName);
    std::string temp_nodeName = nodeName;
    while (k_id_check_set.find(k_id) != k_id_check_set.end())
    {
      temp_nodeName = temp_nodeName + to_string(i);
      k_id = hashNodeName(temp_nodeName);
    }

    kptr_arr[i] = new Kademlia(nodeName, nodeName, k_id, db);
    // std::cout << k_id << "\n";

    k_id_check_set.insert(kptr_arr[i]->GetKId());
    set_data_store(nodeName, "/prefix/data/store/" + k_id, kptr_arr[i]);
  }

  //設定K桶，目前以名字相近的四個節點為K桶
  for (int i = 0; i < NodeNumber; i++)
  {
    if (!set_kbucket_bool)
    {
      break;
    }

    int targetNode = i- Kbuk_Number/2;
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

  //設定合作商家
  for (int i = 0; i < NodeNumber; i++)
  {
    std::set<int> shopSet;
    int upper_count = i+1;
    int lower_count = i-1;

    while (upper_count % ShopChain_num != 0 && upper_count < NodeNumber)
    {
      shopSet.insert(upper_count);
      upper_count++;
    }

    while (lower_count >= 0 && i % ShopChain_num != 0)
    {
      shopSet.insert(lower_count);
      if (lower_count % ShopChain_num == 0)
      {
        break;
      }
      lower_count--;
    }

    if (int(shopSet.size())+1 != ShopChain_num)
    {
      std::cout << "error: shopSet Init size is " << shopSet.size() << " in node " << i << "\n";
    }

    set_customerApp(OrderNumber, "food/food/food/food/food/", kptr_arr[i], i, shopSet);
  }
  
}

int
main(int argc, char* argv[])
{
  CommandLine cmd;
  cmd.Parse(argc, argv);


  AnnotatedTopologyReader topologyReader("", 1000);
  // topologyReader.SetFileName("/home/nccu108753108/ndnSIM/ns-3/src/ndnSIM/nccu_visualization/nccu_topo50.txt");
  topologyReader.SetFileName("/home/nccu108753108/ndnSIM/ns-3/src/ndnSIM/nccu_visualization/nccu_topo100.txt");
  topologyReader.Read();

  //資料庫測試
  sqlite3 *db;
  char *zErrMsg = 0;
  int rc;
  std::string sqlCommand;

  rc = sqlite3_open("1115_100node.db", &db);

  if( rc ){
     fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
     exit(0);
  }else{
     fprintf(stderr, "Opened database successfully\n");
  }

  /* Create SQL statement */
  //刪除資料庫
  //  sqlCommand = std::string(" ") + "DROP TABLE RECORD;";

  //  /* Execute SQL statement */
  //  rc = sqlite3_exec(db, &sqlCommand[0], callback, 0, &zErrMsg);
  //  if( rc != SQLITE_OK ){
  //  fprintf(stderr, "SQL error: %s\n", zErrMsg);
  //     sqlite3_free(zErrMsg);
  //  }else{
  //     fprintf(stdout, "Table delete successfully\n");
  //  }

  //  sqlCommand = std::string(" ") + "DROP TABLE DATAKEYSET;";

  //  /* Execute SQL statement */
  //  rc = sqlite3_exec(db, &sqlCommand[0], callback, 0, &zErrMsg);
  //  if( rc != SQLITE_OK ){
  //  fprintf(stderr, "SQL error: %s\n", zErrMsg);
  //     sqlite3_free(zErrMsg);
  //  }else{
  //     fprintf(stdout, "Table delete successfully\n");
  //  }

   sqlCommand = std::string(" ") + "CREATE TABLE RECORD(" +
         "NODE           TEXT     NOT NULL," +
         "DATA           TEXT     NOT NULL," +
         "DISTANCE       INT      NOT NULL);";

   /* Execute SQL statement */
   rc = sqlite3_exec(db, &sqlCommand[0], callback, 0, &zErrMsg);
   if( rc != SQLITE_OK ){
   fprintf(stderr, "SQL error: %s\n", zErrMsg);
      sqlite3_free(zErrMsg);
   }else{
      fprintf(stdout, "Table select successfully\n");
   }

   sqlCommand = std::string(" ") + "CREATE TABLE DATAKEYSET(" +
         "NODE           TEXT     NOT NULL," +
         "DATA           TEXT     NOT NULL);" ;

  rc = sqlite3_exec(db, &sqlCommand[0], callback, 0, &zErrMsg);
   if( rc != SQLITE_OK ){
   fprintf(stderr, "SQL error: %s\n", zErrMsg);
      sqlite3_free(zErrMsg);
   }else{
      fprintf(stdout, "Table select successfully\n");
   }

   sqlCommand = std::string(" ") + "CREATE TABLE KBUCKET(" +
         "NODE           TEXT     NOT NULL," +
         "KID           TEXT     NOT NULL);" ;

  rc = sqlite3_exec(db, &sqlCommand[0], callback, 0, &zErrMsg);
   if( rc != SQLITE_OK ){
   fprintf(stderr, "SQL error: %s\n", zErrMsg);
      sqlite3_free(zErrMsg);
   }else{
      fprintf(stdout, "Table select successfully\n");
   }

  std::cout << "RECORD COUNT : ";
  sqlCommand = std::string(" ") + "SELECT COUNT(*) FROM RECORD";

  rc = sqlite3_exec(db, &sqlCommand[0], callback, 0, &zErrMsg);
   if( rc != SQLITE_OK ){
   fprintf(stderr, "SQL error: %s\n", zErrMsg);
      sqlite3_free(zErrMsg);
   }else{
      fprintf(stdout, "Table select successfully\n");
   }
  std::cout << "\n";

  std::cout << "Record duplicate : ";
  sqlCommand = std::string(" ") + "SELECT * FROM RECORD GROUP BY NODE, DATA HAVING count(*)>1";

  rc = sqlite3_exec(db, &sqlCommand[0], callback, 0, &zErrMsg);
   if( rc != SQLITE_OK ){
   fprintf(stderr, "SQL error: %s\n", zErrMsg);
      sqlite3_free(zErrMsg);
   }else{
      fprintf(stdout, "Table select successfully\n");
   }
   std::cout << "\n";

  // Install NDN stack on all nodes
  // 可以設定cs size,cache policy等
  ndn::StackHelper ndnHelper;
  ndnHelper.setCsSize(1);
  ndnHelper.InstallAll();

  // Set BestRoute strategy : best-route
  ndn::StrategyChoiceHelper::InstallAll("/", "/localhost/nfd/strategy/best-route");

  // Installing global routing interface on all nodes
  ndn::GlobalRoutingHelper ndnGlobalRoutingHelper;
  ndnGlobalRoutingHelper.InstallAll();

  //產生資料結構，初始化節點模組
  generate_node(db);
    

  // Calculate and install FIBs
  ndn::GlobalRoutingHelper::CalculateRoutes();

  // Ptr<Node> Fail_node3 = Names::Find<Node>("Node3");
  // Ptr<Node> Fail_node4 = Names::Find<Node>("Node4");
  // Ptr<Node> Fail_node0 = Names::Find<Node>("Node0");
  // Ptr<Node> Fail_node2 = Names::Find<Node>("Node2");
   //Simulator::Schedule(Seconds(10.0), ndn::LinkControlHelper::FailLink, Fail_node3, Fail_node4);
   //Simulator::Schedule(Seconds(3.0), ndn::LinkControlHelper::FailLink, Fail_node4, Fail_node0);
  //  Simulator::Schedule(Seconds(1.0), ndn::LinkControlHelper::FailLink, Fail_node0, Fail_node2);
  //  Simulator::Schedule(Seconds(10.0), ndn::LinkControlHelper::UpLink, Fail_node0, Fail_node2);

  ndn::L3RateTracer::InstallAll("rate-trace.txt", Seconds(1000));
  // L2RateTracer::InstallAll("drop-trace.txt", Seconds(5));
  // ndn::CsTracer::InstallAll("cs-trace.txt", Seconds(1));

  Simulator::Stop(Seconds(StopTime));

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
