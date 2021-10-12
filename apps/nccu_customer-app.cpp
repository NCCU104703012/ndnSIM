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

// customer-app.cpp

#include "nccu_customer-app.hpp"

#include "ns3/ptr.h"
#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/packet.h"
#include "ns3/string.h"
#include "ns3/uinteger.h"

#include "ns3/ndnSIM/helper/ndn-stack-helper.hpp"
#include "ns3/ndnSIM/helper/ndn-fib-helper.hpp"

#include "ns3/random-variable-stream.h"

#include <iostream>
#include <algorithm>
#include <random>
#include <string> 
#include <memory>
#include <unordered_map>

//K桶大小
 int Kbuk_Size = 15;

// 一週期的時間長度
int week = 86400;

//一個Order & MicroOrder Query資料量
int OrderQuery_num = 10;

// Micro service Timeout
int MicroService_Timeout = 10;

NS_LOG_COMPONENT_DEFINE("CustomerApp");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED(CustomerApp);

// register NS-3 type
TypeId
CustomerApp::GetTypeId()
{
  static TypeId tid = 
    TypeId("CustomerApp")
      .SetGroupName("Ndn")
      .SetParent<App>()
      .AddConstructor<CustomerApp>()
      .AddAttribute("Prefix", "Prefix, for which producer has the data", StringValue("/"),
                    MakeNameAccessor(&CustomerApp::m_prefix), ndn::MakeNameChecker())
      .AddAttribute(
         "Postfix",
         "Postfix that is added to the output data (e.g., for adding producer-uniqueness)",
         StringValue("/"), MakeNameAccessor(&CustomerApp::m_postfix), ndn::MakeNameChecker())
      .AddAttribute("PayloadSize", "Virtual payload size for Content packets", UintegerValue(1024),
                    MakeUintegerAccessor(&CustomerApp::m_virtualPayloadSize),
                    MakeUintegerChecker<uint32_t>())
      .AddAttribute("Freshness", "Freshness of data packets, if 0, then unlimited freshness",
                    TimeValue(Seconds(0)), MakeTimeAccessor(&CustomerApp::m_freshness),
                    MakeTimeChecker())
      .AddAttribute(
         "Signature",
         "Fake signature, 0 valid signature (default), other values application-specific",
         UintegerValue(0), MakeUintegerAccessor(&CustomerApp::m_signature),
         MakeUintegerChecker<uint32_t>())
      .AddAttribute("KeyLocator",
                    "Name to be used for key locator.  If root, then key locator is not used",
                    ndn::NameValue(), MakeNameAccessor(&CustomerApp::m_keyLocator), ndn::MakeNameChecker())
      .AddAttribute("NodeName", "string name of node", StringValue(""),
          MakeNameAccessor(&CustomerApp::NodeName), ndn::MakeNameChecker())
      .AddAttribute("Query", "string of query", StringValue(""),
          MakeNameAccessor(&CustomerApp::Query), ndn::MakeNameChecker())
      .AddAttribute("Guest", "string of guest list", StringValue(""),
          MakeNameAccessor(&CustomerApp::Guest_list), ndn::MakeNameChecker())
      .AddAttribute("Record", "assign Record", StringValue(""),
          MakeNameAccessor(&CustomerApp::Record), ndn::MakeNameChecker())
      .AddAttribute("Query_Algorithm", "DataManage or DataManageOrigin", StringValue(""),
          MakeNameAccessor(&CustomerApp::query_algorithm), ndn::MakeNameChecker())
      .AddAttribute(
        "Kademlia",
        "Kademlia struct",
        StringValue(""),
        MakeNameAccessor(&CustomerApp::k_ptr),
        ndn::MakeNameChecker())
        ;

  return tid;
}

void
CustomerApp::SendInterest(ndn::Name prefix, std::string logging, bool freshness){
  auto interest = std::make_shared<ndn::Interest>(prefix);
      
  Ptr<UniformRandomVariable> rand = CreateObject<UniformRandomVariable>();
  interest->setNonce(rand->GetValue(0, std::numeric_limits<uint32_t>::max()));
  interest->setMustBeFresh(freshness);

  interest->setInterestLifetime(ndn::time::seconds(3));

  if (logging.length() != 0)
  {
    NS_LOG_DEBUG(logging << interest->getName());
  }
  // Call trace (for logging purposes)
  m_transmittedInterests(interest, this, m_face);

  m_appLink->onReceiveInterest(*interest);
}

// Processing upon start of the application
void
CustomerApp::StartApplication()
{
  // initialize ndn::App
  ndn::App::StartApplication();

  // Add entry to FIB for `/prefix/sub`
  //ndn::FibHelper::AddRoute(GetNode(), prefix, m_face, 0);
  ndn::FibHelper::AddRoute(GetNode(), m_prefix, m_face, 0);
  //ndn::FibHelper::AddRoute(GetNode(), "/prefix/clothes", m_face, 0);

  Order* O_ptr = GetO_ptr()->getNext();
  //Guest* G_ptr = GetG_ptr();

  while (O_ptr != NULL)
  {
    Simulator::Schedule(Seconds(O_ptr->getTimeStamp()), &CustomerApp::InitSendQuery, this);
    Simulator::Schedule(Seconds(O_ptr->getTimeStamp()+ MicroService_Timeout), &CustomerApp::OrderTimeout, this);
    //std::cout << O_ptr->getTimeStamp() << " ";
    O_ptr = O_ptr->getNext();
  }

  //設定資料產生時間
  std::poisson_distribution<int> poisson_record(100);
  std::default_random_engine generator(GetG_ptr()->getTimeStamp());
  double totalTime = (double)poisson_record(generator);

  std::cout << "Guest Time: " ;
  for (int i = 0; i < 10; i++)
  {
    Simulator::Schedule(Seconds(totalTime), &CustomerApp::InitSendData, this);
    totalTime = totalTime + (double)poisson_record(generator);

    std::cout << totalTime << " / " ;
  }
  std::cout << "\n" ;

  // while (G_ptr != NULL)
  // {
  //   Simulator::Schedule(Seconds(G_ptr->getTimeStamp()), &CustomerApp::InitSendData, this);
  //   G_ptr = G_ptr->getNext();
  // }
  
  std::size_t tempHash = std::hash<std::string>{}(NodeName.toUri());
  dayOff = (tempHash%3);
  std::cout << " dayOff: " <<  dayOff << std::endl;

  //shiftTime: 隨機打散上下線時間
  int shiftTime = tempHash%100;

  for (int i = 0; i < 10; i++)
  {
    //設定開始進行上下線的時間, 以及一週的週期
    int startTime = 3;
    Simulator::Schedule(Seconds(startTime + week*i + week/3*dayOff + shiftTime), &CustomerApp::Node_OffLine, this);
    Simulator::Schedule(Seconds(startTime + week*i + week/3*(dayOff+1) + shiftTime), &CustomerApp::Node_OnLine, this);
  }
  
    
}

// Processing when application is stopped
void
CustomerApp::StopApplication()
{
  // cleanup ndn::App
  ndn::App::StopApplication();
}


void
CustomerApp::OnInterest(std::shared_ptr<const ndn::Interest> interest)
{
  ndn::App::OnInterest(interest);

  NS_LOG_DEBUG("Received Interest packet for " << interest->getName());

   std::string inputString = interest->getName().toUri();

    int head = 0, tail;
    std::string DataName, TargetNode, SourceNode, flag, itemtype, queryType;
    
    for (int i = 0; i < 7; i++)
    {
      head = inputString.find_first_of("/", head);
      tail = inputString.find_first_of("/", head+1);
      std::string temp = inputString.substr(head+1, tail-head-1);

      //std::cout  << temp << std::endl;
      head = tail;

      switch (i)
      {
      case 3:
        TargetNode = temp;
        //NS_LOG_DEBUG("targetnode = " << TargetNode);
        break;
      case 4:
        SourceNode = temp;
        //NS_LOG_DEBUG("sourcenode = " << SourceNode);
        break;
      case 5:
        DataName = temp;
        //NS_LOG_DEBUG("dataname = " << DataName);
        break;
      case 6:
        itemtype = temp;
        //NS_LOG_DEBUG("dataname = " << DataName);
        break;
      }
    }

  //收到K桶更新訊息
  //封包格式 prefix/自己節點/來源節點/dataName(Flag 0 or 1)/itemType
  if (itemtype == "Kbucket_connect")
  {
    std::string flag_connect_handshake = DataName;

    // flag = 1 表示先前送過同意訊息給對方 對方已更新K桶
    if (flag_connect_handshake == "1")
    {
      //針對來源節點對K桶進行更新
      bool update_flag = GetK_ptr()->KBucket_hasNode(SourceNode);
      if (!update_flag)
      {
        //沒有節點存在，需要回送disconnect訊息
        std::string k_buk_string = "NULL";
        ndn::Name interest;
        interest.append("prefix").append("data").append("download").append(SourceNode).append(NodeName).append(k_buk_string).append("Kbucket_disconnect");
        SendInterest(interest, "Kbucket_disconnect", true);
        return;
      }
      else
      {
        //connect成功，進行新節點加入後的資料轉移程序
        std::string trans_list = GetK_ptr()->Transform_Data(TargetNode ,SourceNode);

        std::cout << NodeName << ": ";
        GetK_ptr()->print_Kbucket();

        if (trans_list == "|")
        {
          return;
        }

        int head = 0, tail;
        head = trans_list.find_first_of("|", head);
        tail = trans_list.find_first_of("|", head+1);
        std::string trans_targetNode = trans_list.substr(head+1, tail-head-1);

        while (tail != -1)
        {
          trans_targetNode = trans_list.substr(head+1, tail-head-1);
          ndn::Name interest;
          interest.append("prefix").append("data").append("store").append(SourceNode).append(NodeName).append(trans_targetNode).append("Transform_Data");
          SendInterest(interest, "Transform_Data: ", true);

          head = tail;
          tail = trans_list.find_first_of("|", head+1);
          std::cout << "data transform: " << trans_targetNode << " to Node: " << SourceNode << std::endl;
        }
        
      }
    }
    // flag = 0 表示初次要求建立連接
    else if (flag_connect_handshake == "0")
    {
      //運行演算法，確定是否要加入此來源
      std::string replaced_node = GetK_ptr()->KBucket_update(SourceNode);
      //若加入，則反送flag == 1
      //不加入，不動作or送其他封包
      if (replaced_node != SourceNode)
      {
        flag_connect_handshake = "1";
        ndn::Name interest;
        interest.append("prefix").append("data").append("download").append(SourceNode).append(NodeName).append(flag_connect_handshake).append("Kbucket_connect");
        SendInterest(interest, "Kbucket_connect", true);
      }
      else
      {
        flag_connect_handshake = "-1";
        ndn::Name interest;
        interest.append("prefix").append("data").append("download").append(SourceNode).append(NodeName).append(flag_connect_handshake).append("Kbucket_connect");
        SendInterest(interest, "Kbucket_connect", true);
      }
    
    }
    // flag = -1 表示對方不同意連接
    else if (flag_connect_handshake == "-1")
    {
      GetK_ptr()->KBucket_delete(SourceNode);
    }
    else
    {
      std::cout << "error: Kbucket_connect\n";
    }
    return;
  }

  //收到k桶斷線訊息，並判斷是否從K桶資訊中找出其他節點發送connect
  //封包格式 prefix/自己節點/來源節點/dataName(kbuk_string))/itemType
  if (itemtype == "Kbucket_disconnect")
  {
    int head = 0, tail;
    std::string kbuk_string = DataName;

    GetK_ptr()->KBucket_delete(SourceNode);

    std::cout <<"Disconnect " << NodeName << ": ";
    GetK_ptr()->print_Kbucket();

    head = kbuk_string.find_first_of("_", head);
    tail = kbuk_string.find_first_of("_", head+1);

    while (tail != -1)
    {
      std::string newNode = kbuk_string.substr(head+1, tail-head-1);
      std::string update_string = GetK_ptr()->KBucket_update(newNode);

      if (update_string == "add sourceNode to a NULL" && newNode != TargetNode)
      {
        std::string flag_connect_handshake = "0";
        ndn::Name interest;
        interest.append("prefix").append("data").append("download").append(newNode).append(TargetNode).append(flag_connect_handshake).append("Kbucket_connect");
        SendInterest(interest, "Kbucket_connect", true);
      }

      head = tail;
      tail = kbuk_string.find_first_of("_", head+1);
    }
    return;
  }
  
  

  //收到儲存確認訊息，進行DataSet Changing
  if (itemtype == "Store_complete")
  { 

    std::string updateNode = DataSet_update(DataName);

    if (updateNode == NodeName)
    {
      this->SetDataSet(DataName);
      NS_LOG_DEBUG("DataSet-add " << DataName);
      return;
    }
    
    ndn::Name interest;
    interest.append("prefix").append("data").append("download").append(updateNode).append(NodeName).append(DataName).append("dataSet_update");
    SendInterest(interest, "DataSet_update", true);
    return;
  }
  else if (itemtype == "dataSet_update")
  {
    this->SetDataSet(DataName);
    NS_LOG_DEBUG("Get DataSet update from " << SourceNode << " Data: " << DataName);
    NS_LOG_DEBUG("DataSet-add " << DataName);
    return;
  }
  
  

  //若興趣封包是其他節點的Order委託
  if (itemtype == "serviceQuery")
  {
    //確認是否有來自同一源節點的Micro Order正在處理，有則不須新增，返回並同時滿足即可
    Order* O_ptr = GetO_ptr()->getNext();
    while (O_ptr != NULL)
    {
      if (O_ptr->getSourceNode() == SourceNode && !O_ptr->getTerminate())
      {
        NS_LOG_DEBUG("There is a  Micro order processing for " << SourceNode);
        return;
      }
      else
      {
        O_ptr = O_ptr->getNext();
      }
    }

    //新增order並處理 並註明是來自其他節點的order 後續完成後需回傳至原節點
    Order* newOrder = GetO_ptr()->AddOrder_toTail("MicroOrder_" + DataName + "_" + std::to_string(micro_order_count) , SourceNode, 0, 0);
    micro_order_count++;

    newOrder->setHasSourceNode(true);
    newOrder->setSourceNode(SourceNode);
    SendQuery(newOrder, "food", true);
    return;
  }
  else if (itemtype == "timeout")
  {
    Order* O_ptr = GetO_ptr()->getNext();
    while (O_ptr != NULL)
    {
      if (O_ptr->getSourceNode() == SourceNode && !O_ptr->getTerminate())
      {
        break;
      }
      else
      {
        O_ptr = O_ptr->getNext();
      }
    }

    if (O_ptr == NULL)
    {
      NS_LOG_DEBUG("error: timeout get but orderList is NULL");
      return ;
    }

    ndn::Name returnServiceQuery;
    returnServiceQuery.append("prefix").append("data").append("download").append(O_ptr->getSourceNode()).append(NodeName).append(NodeName).append("food");
    O_ptr->setTerminate(true);
    SendInterest(returnServiceQuery, "MicroService_timeout " + O_ptr->getOrderName() + " ", true);
    
  }
  
  else if(DataName == SourceNode)
  {
    ndn::Name InsName;
    InsName.append("prefix").append("data").append("query").append(SourceNode).append("1").append(DataName).append("MicroService_query").append("NULL").append("NULL");

    SendInterest(InsName, "Micro service done, send interest packet to download it ", true);
  }
  else
  {
    ndn::Name InsName;
    InsName.append("prefix").append("data").append("query").append(SourceNode).append("1").append(DataName).append("Data_query").append("NULL").append("NULL");

    SendInterest(InsName, "Found data, send interest packet to download it ", false);
  }
  

}

// 收到資料,完成Order
void
CustomerApp::OnData(std::shared_ptr<const ndn::Data> data)
{
  NS_LOG_DEBUG("Receiving Data packet for " << data->getName());
  
  std::string inputString = data->getName().toUri();

    int head = 0, tail;
    std::string DataName, TargetNode, SourceNode, flag, itemtype, DataString;
    
    for (int i = 0; i < 8; i++)
    {
      head = inputString.find_first_of("/", head);
      tail = inputString.find_first_of("/", head+1);
      std::string temp = inputString.substr(head+1, tail-head-1);

      //std::cout  << temp << std::endl;
      head = tail;

      switch (i)
      {
      case 3:
        SourceNode = temp;
        //NS_LOG_DEBUG("sourcenode = " << SourceNode);
        break;
      case 4:
        flag = temp;
         //NS_LOG_DEBUG("dataname = " << DataName);
        break;
      case 5:
        DataString = temp;
        //NS_LOG_DEBUG("DataString = " << DataString);
        break;
      case 6:
        itemtype = temp;
        //NS_LOG_DEBUG("itemtype = " << itemtype);
        break;
      }
    }

    Order* O_ptr = GetO_ptr()->getNext();
    
    //遍歷所有order, 滿足所有需要此筆資料的order
    while (O_ptr != NULL)
    {
      if (!O_ptr->getTerminate())
      {
        if (O_ptr->checkDataList(DataString))
        {
          NS_LOG_DEBUG("fulfill-order " << O_ptr->getOrderName() <<" data-name " << DataString);
        }
      }
      O_ptr = O_ptr->getNext();
    }

    O_ptr = GetO_ptr()->getNext();

    //搜尋所有order, 將已滿足order返回或完成
    while (O_ptr != NULL)
    {
      if (O_ptr->checkFulFill() && !O_ptr->getTerminate())
      { 
        if (O_ptr->getHasSourceNode())
        {
          ndn::Name returnServiceQuery;
          returnServiceQuery.append("prefix").append("data").append("download").append(O_ptr->getSourceNode()).append(NodeName).append(O_ptr->getOrderName()).append("food");
          O_ptr->setTerminate(true);

          SendInterest(returnServiceQuery, "MicroService return: ", true);
        }

        //輸出log, 表示Order已經完成
        std::string newRecord = O_ptr->getOrderName();
        NS_LOG_DEBUG("Order-Complete " << newRecord );

        //將已滿足order terminate -> ture
        O_ptr->setTerminate(true);
        O_ptr = O_ptr->getNext();
      }
      else
      {
        O_ptr = O_ptr->getNext();
      }
      
    }
    
}


void
CustomerApp::InitSendQuery(){

  Order* O_ptr= GetO_ptr() ;
  for (int i = 0; i <= query_count; i++)
  {
    O_ptr = O_ptr->getNext();
  }
  query_count++;

  SendQuery(O_ptr, O_ptr->getItemType(), false);

}

void
CustomerApp::InitSendData(){
  // Guest* Gptr_head = new Guest("Guest_Record_Node" + to_string(nodeNum) + "_" + to_string(record_count), totalTime);
  // Guest* G_ptr = GetG_ptr();
  // Guest* newG_ptr = G_ptr->getNext();
  // std::string newRecord = G_ptr->getRecordName();

  // std::ostringstream address;
  // address << newG_ptr;
  // ndn::Name newGuest_ptr; 
  // newGuest_ptr.append(address.str());
  // Guest_list = newGuest_ptr;
  // delete(G_ptr);
  int guest_serialNum = GetG_ptr()->getSerialNum();
  std::string newRecord = GetG_ptr()->getRecordName() + "_" + std::to_string(guest_serialNum);
  guest_serialNum++;
  GetG_ptr()->setSerialNum(guest_serialNum);

  std::size_t hashRecord = std::hash<std::string>{}(newRecord);
  std::string binaryRecord = std::bitset<8>(hashRecord).to_string();
  NS_LOG_DEBUG("Guest coming: " << newRecord );

  ndn::Name temp;
  temp.append("prefix").append("data").append("store").append(NodeName);
  temp.append(NodeName).append(newRecord).append("food");
  

  SendInterest(temp, "", false);
}

void
CustomerApp::OrderTimeout(){
    int serial_num = this->GetSerial_num();
    this->SetSerial_num(this->GetSerial_num()+1);

    Order* targetOrder = GetO_ptr();
    std::set<std::string> shopSet = GetO_ptr()->getShopList();

    while (targetOrder->getSerial_num() != serial_num && targetOrder != NULL)
    {
      targetOrder = targetOrder->getNext();
      if (targetOrder == NULL)
      {
        std::cout << "TargetOrder == NULL !!!!!" <<std::endl;
        return ;
      }
    }

    if (targetOrder->getTerminate())
    {
      std::cout << "TargetOrder is terminate !!!!!" <<std::endl;
      return;
    }
    

    std::set<std::string>::iterator i;
    std::set<std::string> dataList = targetOrder->getDataList();
    for (auto i = dataList.begin(); i != dataList.end(); ++i)
    {
      std::string dataString = *i;
      std::string targetNode;
      targetNode = dataString.substr(dataString.find("/") +1);

      if (shopSet.find(targetNode) == shopSet.end())
      {
        continue;
      }

      ndn::Name prefixInterest;
      prefixInterest.append("prefix").append("data").append("download").append(targetNode).append(NodeName).append(targetOrder->getOrderName()).append("timeout");

      SendInterest(prefixInterest, "Sending Service timeout for ", true);
    }
}

void
CustomerApp::SendQuery(Order* O_ptr, std::string serviceType, bool isOrder_from_otherNode){

  NS_LOG_DEBUG("Start-process-Order " << O_ptr->getOrderName());
  //將terminate設為false
  O_ptr->setTerminate(false);

  std::set<std::string> dataSet = this->GetDataSet();
  std::set<std::string>::iterator i,j;

  std::set<std::string> shopSet = GetO_ptr()->getShopList();

  //從shopSet找出節點 發送service Query
  if (!isOrder_from_otherNode)
  {
    for (i = shopSet.begin(); i != shopSet.end(); ++i)
    {
      std::string dataString = *i;

      //將資料存入Order中
      O_ptr->setDataList(dataString);
      NS_LOG_DEBUG("setDataList " << dataString << " in-order " << O_ptr->getOrderName());

      ndn::Name prefixInterest;
      prefixInterest.append("prefix").append("data").append("download").append(dataString).append(NodeName).append(O_ptr->getOrderName()).append("serviceQuery");

      SendInterest(prefixInterest, "Sending Service Query for ", true);
    }
  }

  //當沒有任何資料可以query時，假設有自家菜單可以滿足，直接生成record
  if (dataSet.begin() == dataSet.end() && !isOrder_from_otherNode && (shopSet.begin() == shopSet.end()))
  {

    O_ptr->setTerminate(true);
    return;
  }
  
  if (dataSet.begin() == dataSet.end() && isOrder_from_otherNode )
  {
    ndn::Name returnServiceQuery;
    returnServiceQuery.append("prefix").append("data").append("download").append(O_ptr->getSourceNode()).append(NodeName).append(NodeName).append("food").append(O_ptr->getOrderName());

    SendInterest(returnServiceQuery, "Order-Complete(No Query Data) ", true);
    return;
  }
  

  //將iterator j 調整到上次Query的位置
  j = dataSet.begin();
  //OrderQuery_location = OrderQuery_location % (dataSet.size());

  for (int l = 0; l < OrderQuery_location ;l++)
  {
    j++;
    if (j == dataSet.end())
    {
      j = dataSet.begin();
    }
  }
  
  for (int l = 0; l < OrderQuery_num; l++) {

    if (dataSet.size() == 0)
    {
      std::cout << "Dataset is empty\n";
      return;
    }

    if (j == dataSet.end())
    {
      j = dataSet.begin();
      OrderQuery_location = 0;
    }
    std::string dataString = *j;

    std::cout << "in dataSet: " << dataString << std::endl;


      //將資料存入Order中
      O_ptr->setDataList(dataString);
      NS_LOG_DEBUG("setDataList " << dataString << " in-order " << O_ptr->getOrderName());

      //Query送出
      std::string itemType;
      itemType = "init";

      if (isOrder_from_otherNode)
      {
        NS_LOG_DEBUG("store-data " << dataString << " in-order " << O_ptr->getOrderName());
      }

      //確認queryData list中沒有這筆資料正在Query，新增data結構來紀錄next hop
      if (query_algorithm.toUri() == "/DataManage")
      {
        ndn::Name temp;
        temp.append("prefix").append("data").append("query").append(NodeName).append("0").append(NodeName);
        temp.append(dataString).append("init").append(dataString).append(std::to_string(time(NULL)));
        SendInterest(temp, "Sending Query for ", true);
      }
      else if (GetK_ptr()->GetQueryItem(dataString) == NULL && query_algorithm.toUri() == "/DataManageOrigin")
      {
        GetK_ptr()->queryList->AddData(dataString, itemType);
        ndn::Name temp;
        temp.append("prefix").append("data").append("query").append(NodeName).append("0").append(NodeName);
        temp.append(dataString).append("init").append(dataString).append(std::to_string(time(NULL)));
        SendInterest(temp, "Sending Query for ", true);
      }
    
    //Query下一個目標
    j++;
    OrderQuery_location++;
  }
}

std::string
CustomerApp::DataSet_update(std::string inputDataName){
  int distance = 0;
  std::string output = NodeName.toUri();
  std::size_t biTemp = std::hash<std::string>{}(inputDataName);
  std::string binaryDataName = std::bitset<8>(biTemp).to_string();
  std::set<std::string> shopSet = GetO_ptr()->getShopList();
  std::set<std::string>::iterator o;

  for (int i = 1; i < 9; i++)
    {
        std::string str1 = binaryDataName.substr(i,1);
        std::string str2 = NodeName.toUri().substr(i,1);
        if (str1.compare(str2))
        {
            distance++;
        }
    }

  for (auto o = shopSet.begin(); o != shopSet.end(); ++o)
  {
    std::string shopName = *o;
    int temp_distance = 0;
    for (int i = 1; i < 9; i++)
    {
         std::string str1 = binaryDataName.substr(i,1);
         std::string str2 = shopName.substr(i,1);
       if (str1.compare(str2))
        {
            temp_distance++;
        }
    }
    if (temp_distance > distance)
    {
      distance = temp_distance;
      output = shopName;
    }
      
  }
  return output;
}

// /prefix/data/download/
void
CustomerApp::Node_OnLine(){
  Kademlia* tempK_ptr = GetK_ptr();
  std::string* K_bucket ;
  K_bucket = tempK_ptr->GetK_bucket();
  // for kbucket
  //   Connect_Kbucket()

 std::string flag_connect_handshake = "0";

  for (int i = 0; i < Kbuk_Size; i++)
  {
    if (K_bucket[i] != "NULL")
    {
      ndn::Name prefixInterest;
      prefixInterest.append("prefix").append("data").append("download").append(K_bucket[i]).append(NodeName).append(flag_connect_handshake).append("Kbucket_connect");

      SendInterest(prefixInterest, "Kbucket_connect: ", true);
    }

  }


  isNodeOnline = true;
}

void
CustomerApp::Node_OffLine(){
  
  Kademlia* tempK_ptr = GetK_ptr();
  std::string* K_bucket ;
  std::string Kbuk_string = "_";
  K_bucket = tempK_ptr->GetK_bucket();

  // for kbucket
  //   Disconnect_Kbucket()

  for (int i = 0; i < Kbuk_Size; i++)
  {
    if (K_bucket[i] != "NULL")
    {
      Kbuk_string = Kbuk_string + K_bucket[i] + "_";
    }

  }

  for (int i = 0; i < Kbuk_Size; i++)
  {
    if (K_bucket[i] != "NULL")
    {
      ndn::Name prefixInterest;
      prefixInterest.append("prefix").append("data").append("download").append(K_bucket[i]).append(NodeName).append(Kbuk_string).append("Kbucket_disconnect");

      SendInterest(prefixInterest, "Kbucket_disconnect: ", true);
    }

  }
    

  isNodeOnline = false;
}

} // namespace ns3
