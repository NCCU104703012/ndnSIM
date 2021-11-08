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

// customer-app.hpp

#ifndef CUSTOMER_APP_H_
#define CUSTOMER_APP_H_

#include "ns3/ndnSIM/apps/ndn-app.hpp"
#include "apps/kademlia.hpp"

#include <stdio.h>
#include <sqlite3.h>

namespace ns3 {

/**
 * @brief A simple customer application
 *
 * This applications demonstrates how to send Interests and respond with Datas to incoming interests
 *
 * When application starts it "sets interest filter" (install FIB entry) for /prefix/sub, as well as
 * sends Interest for this prefix
 *
 * When an Interest is received, it is replied with a Data with 1024-byte fake payload
 */
class CustomerApp : public ndn::App {
public:
  // register NS-3 type "CustomerApp"
  static TypeId
  GetTypeId();

  // (overridden from ndn::App) Processing upon start of the application
  virtual void
  StartApplication();

  // (overridden from ndn::App) Processing when application is stopped
  virtual void
  StopApplication();

  // (overridden from ndn::App) Callback that will be called when Interest arrives
  virtual void
  OnInterest(std::shared_ptr<const ndn::Interest> interest);

  // (overridden from ndn::App) Callback that will be called when Data arrives
  virtual void
  OnData(std::shared_ptr<const ndn::Data> contentObject);

  //送出一筆交易紀錄
  // void
  // SendRecord();

  //輸入前綴 送出興趣封包
  void
  SendInterest(ndn::Name prefix, std::string logging, bool freshness);

  //將初始設定轉換成Order 並排序 會在CustomerApp開始時執行
  void
  InitSendQuery();

  //實際送出record 封包
  void
  InitSendData();

  //根據Order內容 找出符合的資料 送出興趣封包
  void
  SendQuery(Order* O_ptr, std::string serviceType, bool isOrder_from_otherNode);

  //送出興趣封包 通知所有未完成的order ＆ record
  void
  OrderTimeout();

  void
  SetNode_Pointer(Ptr<Node> input){parent_node = input;};

  //節點主動上下線，傳送更新K桶封包
  void
  Node_OnLine();

  void
  Node_OffLine();

  void
  NDN_prefix_connect();

  //將Guest 轉換成指標
  Guest*
  GetG_ptr()
  {
    std::stringstream ss;
    std::string temp = Guest_list.toUri().substr(1);
    ss << temp;
    long long unsigned int i;
    ss >> std::hex >> i;
    Guest *output = reinterpret_cast<Guest *>(i);
    return output;
  };

  //將kademlia struct位址從字串轉成指標
  Kademlia*
  GetK_ptr()
  {
    std::stringstream ss;
    std::string temp = k_ptr.toUri().substr(1);
    ss << temp;
    long long unsigned int i;
    ss >> std::hex >> i;
    Kademlia *output = reinterpret_cast<Kademlia *>(i);
    return output;
  };

  //將Query從 Name 轉換成指標
  Order*
  GetO_ptr()
  {
    std::stringstream ss;
    std::string temp = Query.toUri().substr(1);
    ss << temp;
    long long unsigned int i;
    ss >> std::hex >> i;
    Order *output = reinterpret_cast<Order *>(i);
    return output;
  };


  //從資料庫中找出此節點負責Query的資料
  std::string
  GetDataSet(int queryNum){

    sqlite3 *db = GetK_ptr()->getDBptr();
    char *zErrMsg = 0;
    int rc;
    std::string sqlCommand, sqlOutput; 
    char* command_output = (char *)calloc(500,sizeof(char)) ;
    std::string s = "|";
    strcpy(command_output, &s[0]);

    sqlCommand = std::string("SELECT * from DATAKEYSET WHERE NODE=") + "'" + GetK_ptr()->GetKId() + "'" + " ORDER BY Random() LIMIT " + std::to_string(queryNum)  +  " ;";  
    
    /* Execute SQL statement */
   rc = sqlite3_exec(db, &sqlCommand[0], this->DB_getDATA, &command_output, &zErrMsg);
   if( rc != SQLITE_OK ){
   fprintf(stderr, "SQL error: %s\n", zErrMsg);
      sqlite3_free(zErrMsg);
   }

    // std::cout << "string len = "<< command_output << "\n";
    std::string outputString(command_output);
    free(command_output);

    return outputString;
  };

  //將資料加入此節點負責Query行列
  void
  SetDataSet(std::string inputData){
    dataSet.insert(inputData);

    sqlite3 *db = GetK_ptr()->getDBptr();
    char *zErrMsg = 0;
    int rc;
    std::string sqlCommand, sqlOutput; 

    sqlCommand = std::string("INSERT INTO DATAKEYSET (NODE,DATA)") +
                 "VALUES('"+ GetK_ptr()->GetKId() + "', '" + inputData + "');";
    
    /* Execute SQL statement */
   rc = sqlite3_exec(db, &sqlCommand[0], this->DB_setDATA, 0, &zErrMsg);
   if( rc != SQLITE_OK ){
   fprintf(stderr, "SQL error: %s\n", zErrMsg);
      sqlite3_free(zErrMsg);
   }
    
  }

  int
  GetSerial_num(){return serial_num;};

  void
  SetSerial_num(int input){serial_num = input;};

  //確認ShopSet中有沒有更近的節點，進行DataSet更新
  std::string
  DataSet_update(std::string inputDataName);

  void
  SetKubcket_init(){
    GetK_ptr()->Init_Kbucket();
    GetK_ptr()->print_Kbucket();
  }

  void
  Store_kbucket();


private:
  //void
  //SendInterest();

  static int DB_getDATA(void *NotUsed, int argc, char **argv, char **azColName){
      
      
      char **result_str = (char **)NotUsed;
      std::string s = "|";
      strcat(*result_str,argv[1]);
      strcat(*result_str, &s[0]);

      // std::cout << "success get data : " << argv[0] << " " << argv[1] << "\n";
      return 0;
  }

  static int DB_setDATA(void *NotUsed, int argc, char **argv, char **azColName){
      // std::cout << "success get data : " << argv[0] << " " << argv[1] << "\n";
      return 0;
  }


  ndn::Name m_prefix;
  ndn::Name m_postfix;
  uint32_t m_virtualPayloadSize;
  Time m_freshness;

  uint32_t m_signature;
  ndn::Name m_keyLocator;

  Ptr<Node> parent_node ;

  //本身節點的交易紀錄 用來送出給Data store
  ndn::Name Record;
  int record_count = 0;

  //用來生成新record string的編號 本身意義不大
  int micro_order_count = 0;

  //用來製作興趣封包
  ndn::Name NodeName;

  //指定欲儲存的節點
  ndn::Name TargetNode;

  //送出Query使用
  ndn::Name Query;
  int query_count = 0;

  //k_ptr內容為"/0x00000" 不為純位址 要注意
  ndn::Name k_ptr;

  //儲存商家清單 已知的資料清單 用來生成order需要query的項目
  std::set<std::string> dataSet = {};

  //儲存poisson進入的顧客 用來生成record
  ndn::Name Guest_list;

  //使用何種演算法進行 Data query
  ndn::Name query_algorithm;

  //紀錄休息的日期（七分之一時間休息）
  int dayOff = -1;

  //紀錄目前節點上下線狀態
  bool isNodeOnline = true;

  //紀錄目前Query資料的位置
  int OrderQuery_location = 0;

  int serial_num = 1;

};

} // namespace ns3

#endif // CUSTOMER_APP_H_
