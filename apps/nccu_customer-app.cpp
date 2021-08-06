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
#include <string> 
#include <memory>
#include <unordered_map>

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
  Guest* G_ptr = GetG_ptr();

  while (O_ptr != NULL)
  {
    Simulator::Schedule(Seconds(O_ptr->getTimeStamp()), &CustomerApp::InitSendQuery, this);
    Simulator::Schedule(Seconds(O_ptr->getTimeStamp()+ 2), &CustomerApp::OrderTimeout, this);
    std::cout << O_ptr->getTimeStamp() << " ";
    O_ptr = O_ptr->getNext();
  }

  while (G_ptr != NULL)
  {
    Simulator::Schedule(Seconds(G_ptr->getTimeStamp()), &CustomerApp::InitSendData, this);
    G_ptr = G_ptr->getNext();
  }
  
  std::cout << std::endl;

}

// Processing when application is stopped
void
CustomerApp::StopApplication()
{
  // cleanup ndn::App
  ndn::App::StopApplication();
}

// void
// CustomerApp::SendInterest()
// {
//   /////////////////////////////////////
//   // Sending one Interest packet out //
//   /////////////////////////////////////


//   // Create and configure ndn::Interest
//   auto interest = std::make_shared<ndn::Interest>(m_prefix);
//   Ptr<UniformRandomVariable> rand = CreateObject<UniformRandomVariable>();
//   interest->setNonce(rand->GetValue(0, std::numeric_limits<uint32_t>::max()));

//   interest->setInterestLifetime(ndn::time::seconds(3));
//   //interest->setDefaultCanBePrefix(true);

//   NS_LOG_DEBUG("Sending Interest packet for " << *interest);

//   // Call trace (for logging purposes)
//   m_transmittedInterests(interest, this, m_face);

//   m_appLink->onReceiveInterest(*interest);
// }

// Callback that will be called when Interest arrives
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

  //收到儲存確認訊息，進行DataSet Changing
  if (itemtype == "Store_complete")
  {
    std::string updateNode = DataSet_update(DataName);

    if (updateNode == NodeName)
    {
      this->SetDataSet("food/" + DataName);
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
    this->SetDataSet("food/" + DataName);
    NS_LOG_DEBUG("Get DataSet update from " << SourceNode << " Data: " << DataName);
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
    Order* newOrder = GetO_ptr()->AddOrder_toTail("MicroOrder_" + DataName, SourceNode, 0, 0);
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
    InsName.append("prefix").append("data").append("query").append(SourceNode).append("1").append(DataName).append(itemtype);

    SendInterest(InsName, "Micro service done, send interest packet to download it ", true);
  }
  else
  {
    ndn::Name InsName;
    InsName.append("prefix").append("data").append("query").append(SourceNode).append("1").append(DataName).append(itemtype);

    SendInterest(InsName, "Found data, send interest packet to download it ", false);
  }
  

}

// Callback that will be called when Data arrives
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
        if (O_ptr->checkDataList(DataString, itemtype))
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


//送出一筆交易紀錄
void
CustomerApp::SendRecord()
{
  //record分割

  int head = 0 , tail = Record.toUri().find_first_of("/");
  std::string record_output;
  for (int i = 0; i <= record_count; i++)
  {
    head = tail+1;
    tail = Record.toUri().find_first_of("/",head);
  }
  
  record_output = Record.toUri().substr(head, tail-head);

  //將record加入dataSet中
  this->SetDataSet("food/" + record_output);

  //生成興趣封包
  std::size_t hashRecord = std::hash<std::string>{}(record_output);
  std::string binaryRecord = std::bitset<8>(hashRecord).to_string();
  NS_LOG_DEBUG("hash Record for " << binaryRecord << " " << record_output);

  ndn::Name temp;
  temp.append("prefix").append("data").append("store").append(NodeName);
  temp.append(GetK_ptr()->GetNext_Node(binaryRecord)->GetKId()).append(record_output).append("food");
  record_count++;

  SendInterest(temp, "Sending Record for ", true);
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
  Guest* G_ptr = GetG_ptr();
  Guest* newG_ptr = G_ptr->getNext();
  std::string newRecord = G_ptr->getRecordName();

  std::ostringstream address;
  address << newG_ptr;
  ndn::Name newGuest_ptr; 
  newGuest_ptr.append(address.str());
  Guest_list = newGuest_ptr;
  delete(G_ptr);

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
    for (i = dataList.begin(); i != dataList.end(); ++i)
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
  std::set<std::string>::iterator i;

  std::set<std::string> shopSet = GetO_ptr()->getShopList();

  //從shopSet找出節點 發送service Query
  if (!isOrder_from_otherNode)
  {
    for (i = shopSet.begin(); i != shopSet.end(); ++i)
    {
      std::string dataString = *i;

      //將資料存入Order中
      O_ptr->setDataList("food/" + dataString);
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

  }
  
  if (dataSet.begin() == dataSet.end() && isOrder_from_otherNode )
  {
    ndn::Name returnServiceQuery;
    returnServiceQuery.append("prefix").append("data").append("download").append(O_ptr->getSourceNode()).append(NodeName).append(NodeName).append("food").append(O_ptr->getOrderName());

    SendInterest(returnServiceQuery, "Order-Complete ", true);
    return;
  }

  for (i = dataSet.begin(); i != dataSet.end(); ++i) {
    std::string dataString = *i;
    if (dataString.find(serviceType) >= 0)
    {
      //將資料存入Order中
      O_ptr->setDataList(dataString);
      NS_LOG_DEBUG("setDataList " << dataString << " in-order " << O_ptr->getOrderName());

      //Query送出
      std::string query_output, itemType;
      query_output = dataString.substr(dataString.find_first_of("/"), dataString.size()-dataString.find_first_of("/"));
      itemType = dataString.substr(0, dataString.find_first_of("/"));

      if (isOrder_from_otherNode)
      {
        NS_LOG_DEBUG("store-data " << query_output << " in-order " << O_ptr->getOrderName());
      }

      ndn::Name temp;
      temp.append("prefix").append("data").append("query").append(NodeName).append("0").append(NodeName);
      temp.append(query_output).append(itemType).append(query_output + std::to_string(time(NULL)));
      
      SendInterest(temp, "Sending Query for ", true);
    }
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

    for ( o = shopSet.begin(); o != shopSet.end(); ++o)
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


} // namespace ns3
