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

#include "data_management.hpp"

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
#include <string> 
#include <memory>


bool ndnFault_tolerant_disable = false;


NS_LOG_COMPONENT_DEFINE("DataManage");


namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED(DataManage);

// register NS-3 type
TypeId
DataManage::GetTypeId()
{
  static TypeId tid = 
    TypeId("DataManage")
      .SetGroupName("Ndn")
      .SetParent<App>()
      .AddConstructor<DataManage>()
      .AddAttribute("Prefix", "Prefix, for which producer has the data", StringValue("/"),
                    MakeNameAccessor(&DataManage::m_prefix), ndn::MakeNameChecker())
      .AddAttribute(
         "Postfix",
         "Postfix that is added to the output data (e.g., for adding producer-uniqueness)",
         StringValue("/"), MakeNameAccessor(&DataManage::m_postfix), ndn::MakeNameChecker())
      .AddAttribute("PayloadSize", "Virtual payload size for Content packets", UintegerValue(1024),
                    MakeUintegerAccessor(&DataManage::m_virtualPayloadSize),
                    MakeUintegerChecker<uint32_t>())
      .AddAttribute("Freshness", "Freshness of data packets, if 0, then unlimited freshness",
                    TimeValue(Seconds(0)), MakeTimeAccessor(&DataManage::m_freshness),
                    MakeTimeChecker())
      .AddAttribute(
         "Signature",
         "Fake signature, 0 valid signature (default), other values application-specific",
         UintegerValue(0), MakeUintegerAccessor(&DataManage::m_signature),
         MakeUintegerChecker<uint32_t>())
      .AddAttribute("KeyLocator",
                    "Name to be used for key locator.  If root, then key locator is not used",
                    ndn::NameValue(), MakeNameAccessor(&DataManage::m_keyLocator), ndn::MakeNameChecker())
      .AddAttribute("Query", "string of query", StringValue(""),
          MakeNameAccessor(&DataManage::Query), ndn::MakeNameChecker())
      .AddAttribute(
        "Kademlia",
        "Kademlia struct",
        StringValue(""),
        MakeNameAccessor(&DataManage::k_ptr),
        ndn::MakeNameChecker())
      ;

  return tid;
}

// Processing upon start of the application
void
DataManage::StartApplication()
{
  // initialize ndn::App
  ndn::App::StartApplication();

  // Add entry to FIB for `/prefix/sub`
  //ndn::FibHelper::AddRoute(GetNode(), prefix, m_face, 0);
  ndn::FibHelper::AddRoute(GetNode(), m_prefix, m_face, 0);
  //ndn::FibHelper::AddRoute(GetNode(), "/prefix/clothes", m_face, 0);

  // Schedule send of first interest
}

// Processing when application is stopped
void
DataManage::StopApplication()
{
  // cleanup ndn::App
  ndn::App::StopApplication();
}

void
DataManage::SendInterest()
{
  /////////////////////////////////////
  // Sending one Interest packet out //
  /////////////////////////////////////


  // Create and configure ndn::Interest
  auto interest = std::make_shared<ndn::Interest>(m_prefix);
  Ptr<UniformRandomVariable> rand = CreateObject<UniformRandomVariable>();
  interest->setNonce(rand->GetValue(0, std::numeric_limits<uint32_t>::max()));
  interest->setMustBeFresh(1);
  interest->setInterestLifetime(ndn::time::seconds(3));


  NS_LOG_DEBUG("Sending Interest packet for " << *interest);

  // Call trace (for logging purposes)
  m_transmittedInterests(interest, this, m_face);

  m_appLink->onReceiveInterest(*interest);
}

// Callback that will be called when Interest arrives
void
DataManage::OnInterest(std::shared_ptr<const ndn::Interest> interest)
{
  ndn::App::OnInterest(interest);

  NS_LOG_DEBUG("Received Interest packet for " << interest->getName());

  std::string inputString = interest->getName().toUri();

    int head = 0, tail;
    std::string DataName, TargetNode, SourceNode, flag, itemType;
    ndn::Name outInterest;


    
    for (int i = 0; i < 8; i++)
    {
      head = inputString.find("/", head);
      tail = inputString.find("/", head+1);
      std::string temp = inputString.substr(head+1, tail-head-1);

      //std::cout  << temp << std::endl;
      head = tail;

      switch (i)
      {
      case 3:
        TargetNode = temp;
        //NS_LOG_DEBUG("targetnode = " << TargetNode);
      case 4:
        flag = temp;
        break;
      case 5:
        SourceNode = temp;
        //NS_LOG_DEBUG("sourcenode = " << SourceNode);
        break;
      case 6:
        DataName = temp;
        //NS_LOG_DEBUG("dataname = " << DataName);
        break;
      case 7:
        itemType = temp;
        //NS_LOG_DEBUG("dataname = " << DataName);
        break;
      }
    }

    //檢查是否有前綴異常
    if (TargetNode.substr(0,4) != GetK_ptr()->GetKId().substr(0,4))
    {
      std::cout << "error: prefix forward four bits not same " << TargetNode << "|" << GetK_ptr()->GetKId() << "\n";
      return;
    }
    

    std::size_t biTemp = std::hash<std::string>{}(DataName);
    std::string binaryDataName = std::bitset<8>(biTemp).to_string();

    NS_LOG_DEBUG("Query data " << DataName << "  Hash: " << binaryDataName);

    //上下線狀態判定
    if (!GetK_ptr()->GetisOnline())
    {
      if (TargetNode == GetK_ptr()->GetKId())
      {
        NS_LOG_DEBUG("error! this node is offline : " << interest->getName());
        return;
      }

      std::string TargetNode = ndnFault_tolerant(binaryDataName);

      if (TargetNode == "NULL")
      {
        //std::cout << "******************" << std::endl;
        NS_LOG_DEBUG("NO-match-Data-&-next-Node");
        //std::cout << "******************" << std::endl;
        return;
      }

      outInterest.append("prefix").append("data").append("query").append(TargetNode).append("0").append(SourceNode).append(DataName).append(itemType);

      // Create and configure ndn::Interest
      auto interest = std::make_shared<ndn::Interest>(outInterest);
      Ptr<UniformRandomVariable> rand = CreateObject<UniformRandomVariable>();
      interest->setNonce(rand->GetValue(0, std::numeric_limits<uint32_t>::max()));
      interest->setMustBeFresh(1);
      interest->setInterestLifetime(ndn::time::seconds(1));
      interest->setHopLimit(20);

      NS_LOG_DEBUG("Query another Node for ndnFault_tolerant(offline) " << *interest);

      // Call trace (for logging purposes)
      m_transmittedInterests(interest, this, m_face);

      m_appLink->onReceiveInterest(*interest);

      return;
    }

    TargetNode = GetK_ptr()->GetKId();


    //確認flag 若為1則為match到的source節點傳來興趣封包
    if (flag.compare("1") == 0)
    {
      Order* Optr = GetO_ptr()->getNext();
      Order* preOrder = GetO_ptr();
      while (Optr != NULL)
      {
        if (Optr->getSourceNode() == SourceNode && Optr->getTerminate())
        {
          Optr->deleteOrder(preOrder);
          Optr = NULL;
        }
        else
        {
          preOrder = Optr;
          Optr = Optr->getNext();
        }
      }

      ndn::Name outData;
      outData.append("prefix").append("data").append("download").append(SourceNode).append(TargetNode).append(DataName).append(itemType);

      auto data = std::make_shared<ndn::Data>(interest->getName());
      data->setFreshnessPeriod(ndn::time::milliseconds(1000));
      data->setContent(std::make_shared< ::ndn::Buffer>(1024));
      ndn::StackHelper::getKeyChain().sign(*data);

      NS_LOG_DEBUG("Sending Data packet for " << data->getName());

      // Call trace (for logging purposes)
      m_transmittedDatas(data, this, m_face);

      m_appLink->onReceiveData(*data);
    }

    //確認是否有此資料 若無則從k桶找尋下一目標
    else if(GetK_ptr()->GetData(DataName))
    {
      outInterest.append("prefix").append("data").append("download").append(SourceNode).append(TargetNode).append(DataName).append(itemType);

      // Create and configure ndn::Interest
      auto interest = std::make_shared<ndn::Interest>(outInterest);
      Ptr<UniformRandomVariable> rand = CreateObject<UniformRandomVariable>();
      interest->setNonce(rand->GetValue(0, std::numeric_limits<uint32_t>::max()));
      interest->setMustBeFresh(1);
      interest->setInterestLifetime(ndn::time::seconds(1));

      NS_LOG_DEBUG("Sending Interest packet for " << *interest);

      // Call trace (for logging purposes)
      m_transmittedInterests(interest, this, m_face);

      m_appLink->onReceiveInterest(*interest);
    }
    else
    {
      //從K桶找下一目標 目前用預設第一個節點

      if (GetK_ptr()->GetNext_Node(binaryDataName, 1, SourceNode) == GetK_ptr()->GetKId())
      {
        std::string TargetNode = ndnFault_tolerant(binaryDataName);

        if (TargetNode == "NULL" || ndnFault_tolerant_disable)
        {
          //std::cout << "******************" << std::endl;
          NS_LOG_DEBUG("NO-match-Data-&-next-Node");
          //std::cout << "******************" << std::endl;
          return;
        }
        
        outInterest.append("prefix").append("data").append("query").append(TargetNode).append("0").append(SourceNode).append(DataName).append(itemType);

        // Create and configure ndn::Interest
        auto interest = std::make_shared<ndn::Interest>(outInterest);
        Ptr<UniformRandomVariable> rand = CreateObject<UniformRandomVariable>();
        interest->setNonce(rand->GetValue(0, std::numeric_limits<uint32_t>::max()));
        interest->setMustBeFresh(1);
        interest->setInterestLifetime(ndn::time::seconds(1));
        interest->setHopLimit(20);

        NS_LOG_DEBUG("Query another Node for ndnFault_tolerant " << *interest);

        // Call trace (for logging purposes)
        m_transmittedInterests(interest, this, m_face);

        m_appLink->onReceiveInterest(*interest);

        return;
      }
      else
      {
        TargetNode = GetK_ptr()->GetNext_Node(binaryDataName, 1, SourceNode);
        outInterest.append("prefix").append("data").append("query").append(TargetNode).append("0").append(SourceNode).append(DataName).append(itemType);

        // Create and configure ndn::Interest
        auto interest = std::make_shared<ndn::Interest>(outInterest);
        Ptr<UniformRandomVariable> rand = CreateObject<UniformRandomVariable>();
        interest->setNonce(rand->GetValue(0, std::numeric_limits<uint32_t>::max()));
        interest->setMustBeFresh(1);
        interest->setInterestLifetime(ndn::time::seconds(1));

        NS_LOG_DEBUG( "NodeID: " << GetK_ptr()->GetKId() << " Query another Node for data " << *interest);

        // Call trace (for logging purposes)
        m_transmittedInterests(interest, this, m_face);

        m_appLink->onReceiveInterest(*interest);
      }
    }
}

// Callback that will be called when Data arrives
void
DataManage::OnData(std::shared_ptr<const ndn::Data> data)
{
  NS_LOG_DEBUG("Receiving Data packet for " << data->getName());

  std::cout << "DATA received for name " << data->getName() << std::endl;
}

void
DataManage::SetNode_Pointer(Ptr<Node> input)
{
  parent_node = input;
}


//NDN錯誤容忍，輸出下一個目標前綴
std::string
DataManage::ndnFault_tolerant(std::string DataName)
{
  if(DataName.length() != 8){
    std::cout << "error: ndnFault_tolerant DataName < 8\n";
    return "NULL";
  }

  int sameBits = 0, careBits = 1;
  std::string TargetNode ;

  for (int i = 0; i < 8; i++)
    {
        std::string str1 = std::to_string((GetK_ptr()->GetKId())[i]);
        std::string str2 = std::to_string(DataName[i]);
        if (str1.compare(str2) == 0)
        {
          sameBits++;
        }
        else
        {
          break;
        }
    }
  
  if (sameBits == 0){ sameBits++; }
  else if (sameBits >= 8){ return "NULL"; }

  careBits = 8 / 2;
  
  while (8 - careBits <= sameBits)
  {
    careBits = careBits / 2;
  }

  std::cout << "Node k-id : " + GetK_ptr()->GetKId() + "dataName: " + DataName + "\n"; 

  TargetNode = DataName.substr(0,(8 - careBits));

  for (int i = 0; i < careBits; i++)
  {
    TargetNode = TargetNode + "x";
  }
  
  if(TargetNode.length() != 8){
    std::cout << "error: ndnFault_tolerant output DataName != 8  : " << TargetNode << "  careBits = " << std::to_string(careBits)  <<  "\n" ;
    return "NULL";
  }

  return TargetNode;
}


} // namespace ns3