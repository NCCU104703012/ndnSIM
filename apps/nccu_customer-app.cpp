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
CustomerApp::SendInterest(ndn::Name prefix, std::string logging){
  auto interest = std::make_shared<ndn::Interest>(prefix);
      
  Ptr<UniformRandomVariable> rand = CreateObject<UniformRandomVariable>();
  interest->setNonce(rand->GetValue(0, std::numeric_limits<uint32_t>::max()));

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

  // Schedule send of first interest
  // Simulator::Schedule(Seconds(1.0), &CustomerApp::SendRecord, this);
  // Simulator::Schedule(Seconds(1.5), &CustomerApp::SendRecord, this);
  // Simulator::Schedule(Seconds(2.0), &CustomerApp::SendRecord, this);
  // Simulator::Schedule(Seconds(2.5), &CustomerApp::SendRecord, this);
  // Simulator::Schedule(Seconds(3.0), &CustomerApp::SendRecord, this);

  // Simulator::Schedule(Seconds(4), &CustomerApp::SendQuery, this);
  // Simulator::Schedule(Seconds(7), &CustomerApp::SendQuery, this);
  // Simulator::Schedule(Seconds(10), &CustomerApp::SendQuery, this);
  // Simulator::Schedule(Seconds(13), &CustomerApp::SendQuery, this);
  
  // Simulator::Schedule(Seconds(1), &CustomerApp::SendQuery_shop, this);

  Order* O_ptr = GetO_ptr()->getNext();
  // for (int i = 0; i < GetO_ptr()->getTargetNum() ; i++)
  // {
  //   Simulator::Schedule(Seconds(O_ptr->getTimeStamp()), &CustomerApp::SendQuery, this);
  //   std::cout << O_ptr->getTimeStamp() << std::endl;
  //   if (O_ptr->getNext() != NULL)
  //   {
  //     O_ptr = O_ptr->getNext();
  //   }
  // }

  while (O_ptr != NULL)
  {
    Simulator::Schedule(Seconds(O_ptr->getTimeStamp()), &CustomerApp::InitSendQuery, this);
    Simulator::Schedule(Seconds(O_ptr->getTimeStamp()+0.01), &CustomerApp::OrderTimeout, this);
    std::cout << O_ptr->getTimeStamp() << " ";
    O_ptr = O_ptr->getNext();
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

void
CustomerApp::SendInterest()
{
  /////////////////////////////////////
  // Sending one Interest packet out //
  /////////////////////////////////////


  // Create and configure ndn::Interest
  auto interest = std::make_shared<ndn::Interest>(m_prefix);
  Ptr<UniformRandomVariable> rand = CreateObject<UniformRandomVariable>();
  interest->setNonce(rand->GetValue(0, std::numeric_limits<uint32_t>::max()));

  interest->setInterestLifetime(ndn::time::seconds(3));
  //interest->setDefaultCanBePrefix(true);

  NS_LOG_DEBUG("Sending Interest packet for " << *interest);

  // Call trace (for logging purposes)
  m_transmittedInterests(interest, this, m_face);

  m_appLink->onReceiveInterest(*interest);
}

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

  if (itemtype == "Store_complete")
  {
    this->SetDataSet("food/" + DataName);
    NS_LOG_DEBUG("DataSet-add " << DataName);
    return;
  }
  

  //若興去封包是其他節點的Order委託
  if (itemtype == "serviceQuery")
  {
    //新增order並處理 並註明是來自其他節點的order 後續完成後需回傳至原節點
    //std::cout<< "input packet is service query!" << std::endl;
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
    SendInterest(returnServiceQuery, "MicroService return for timeout!!");
    
  }
  
  else
  {
    ndn::Name InsName;
    InsName.append("prefix").append("data").append("query").append(SourceNode).append("1").append(TargetNode).append(DataName).append(itemtype);

    SendInterest(InsName, "Found data, send interest packet to download it ");
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
        DataString = temp;
        //NS_LOG_DEBUG("DataString = " << DataString);
        break;
      case 7:
        itemtype = temp;
        //NS_LOG_DEBUG("itemtype = " << itemtype);
        break;
      }
    }

    Order* O_ptr = GetO_ptr()->getNext();
    
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

    while (O_ptr != NULL)
    {
      if (O_ptr->checkFulFill() && !O_ptr->getTerminate())
      { 
        if (O_ptr->getHasSourceNode())
        {
          ndn::Name returnServiceQuery;
          //returnServiceQuery.append("prefix").append("service").append("return").append(O_ptr->getSourceNode()).append(NodeName);
          returnServiceQuery.append("prefix").append("data").append("download").append(O_ptr->getSourceNode()).append(NodeName).append(O_ptr->getOrderName()).append("food");

          // auto data = std::make_shared<ndn::Data>(returnServiceQuery);
          // data->setFreshnessPeriod(ndn::time::milliseconds(1000));
          // data->setContent(std::make_shared< ::ndn::Buffer>(1024));
          // ndn::StackHelper::getKeyChain().sign(*data);

          // NS_LOG_DEBUG("Sending Data packet for return Service" << data->getName());

          // // Call trace (for logging purposes)
          // m_transmittedDatas(data, this, m_face);

          // m_appLink->onReceiveData(*data);
          O_ptr->setTerminate(true);
          SendInterest(returnServiceQuery, "MicroService return!!");
        }
        else
        {
          //生成一筆新的紀錄 並送出儲存
          std::string newRecord = O_ptr->getOrderName();

          //this->SetDataSet("food/" + newRecord);

          std::size_t hashRecord = std::hash<std::string>{}(newRecord);
          std::string binaryRecord = std::bitset<8>(hashRecord).to_string();
          NS_LOG_DEBUG("Order-Complete " << newRecord );

          ndn::Name temp;
          temp.append("prefix").append("data").append("store").append(NodeName);
          temp.append(NodeName).append(newRecord).append("food");
          new_record_count++;

          SendInterest(temp, "Sending Record for ");
        }

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

  SendInterest(temp, "Sending Record for ");
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

      SendInterest(prefixInterest, "Sending Service timeout for ");
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

      SendInterest(prefixInterest, "Sending Service Query for ");
    }
  }

  //當沒有任何資料可以query時，假設有自家菜單可以滿足，直接生成record
  if (dataSet.begin() == dataSet.end() && !isOrder_from_otherNode && (shopSet.begin() == shopSet.end()))
  {
    std::string newRecord = O_ptr->getOrderName();

    this->SetDataSet("food/" + newRecord);

    std::size_t hashRecord = std::hash<std::string>{}(newRecord);
    std::string binaryRecord = std::bitset<8>(hashRecord).to_string();
    NS_LOG_DEBUG("hash Record for " << binaryRecord << " " << newRecord);

    ndn::Name temp;
    temp.append("prefix").append("data").append("store").append(NodeName);
    temp.append(GetK_ptr()->GetNext_Node(binaryRecord)->GetKId()).append(newRecord).append("food");
    new_record_count++;

    SendInterest(temp, "Sending Record for ");

    //將已滿足order terminate -> ture
    O_ptr->setTerminate(true);

  }
  
  if (dataSet.begin() == dataSet.end() && isOrder_from_otherNode )
  {
    ndn::Name returnServiceQuery;
    returnServiceQuery.append("prefix").append("data").append("download").append(O_ptr->getSourceNode()).append(NodeName).append(NodeName).append("food").append(O_ptr->getOrderName());

    SendInterest(returnServiceQuery, "Return service for ");
    return;
  }

  for (i = dataSet.begin(); i != dataSet.end(); ++i) {
    std::string dataString = *i;
    if (dataString.find(serviceType) >= 0)
    {
      //將資料存入Order中
      O_ptr->setDataList(dataString);

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
      
      SendInterest(temp, "Sending Query for ");
    }
  }

  
  
  
  
}


} // namespace ns3
