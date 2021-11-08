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

#include "data_store.hpp"

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

NS_LOG_COMPONENT_DEFINE("DataStore");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED(DataStore);

// register NS-3 type
TypeId
DataStore::GetTypeId()
{
  static TypeId tid = 
    TypeId("DataStore")
      .SetGroupName("Ndn")
      .SetParent<App>()
      .AddConstructor<DataStore>()
      .AddAttribute("Prefix", "Prefix, for which producer has the data", StringValue("/"),
                    MakeNameAccessor(&DataStore::m_prefix), ndn::MakeNameChecker())
      .AddAttribute(
         "Postfix",
         "Postfix that is added to the output data (e.g., for adding producer-uniqueness)",
         StringValue("/"), MakeNameAccessor(&DataStore::m_postfix), ndn::MakeNameChecker())
      .AddAttribute("PayloadSize", "Virtual payload size for Content packets", UintegerValue(1024),
                    MakeUintegerAccessor(&DataStore::m_virtualPayloadSize),
                    MakeUintegerChecker<uint32_t>())
      .AddAttribute("Freshness", "Freshness of data packets, if 0, then unlimited freshness",
                    TimeValue(Seconds(0)), MakeTimeAccessor(&DataStore::m_freshness),
                    MakeTimeChecker())
      .AddAttribute(
         "Signature",
         "Fake signature, 0 valid signature (default), other values application-specific",
         UintegerValue(0), MakeUintegerAccessor(&DataStore::m_signature),
         MakeUintegerChecker<uint32_t>())
      .AddAttribute("KeyLocator",
                    "Name to be used for key locator.  If root, then key locator is not used",
                    ndn::NameValue(), MakeNameAccessor(&DataStore::m_keyLocator), ndn::MakeNameChecker())
      .AddAttribute(
        "Kademlia",
        "Kademlia struct",
        StringValue(""),
        MakeNameAccessor(&DataStore::k_ptr),
        ndn::MakeNameChecker()
      );

  return tid;
}

// Processing upon start of the application
void
DataStore::StartApplication()
{
  // initialize ndn::App
  ndn::App::StartApplication();

  // Add entry to FIB for `/prefix/sub`
  ndn::FibHelper::AddRoute(GetNode(), m_prefix, m_face, 0);

  // Schedule send of first interest

  //Simulator::Schedule(Seconds(1.0), &DataStore::SendInterest, this);
  //Simulator::Schedule(Seconds(20.0), &DataStore::SendInterest, this);

}

// Processing when application is stopped
void
DataStore::StopApplication()
{
  // cleanup ndn::App
  //ndn::App::StopApplication();
}

void
DataStore::SendInterest()
{
  /////////////////////////////////////
  // Sending one Interest packet out //
  /////////////////////////////////////
  std::string temp = m_prefix.toUri();

  ndn::Name output("/prefix/data/store/6");


  // Create and configure ndn::Interest
  auto interest = std::make_shared<ndn::Interest>(output.append(std::to_string(packet_count)));
  Ptr<UniformRandomVariable> rand = CreateObject<UniformRandomVariable>();
  interest->setNonce(rand->GetValue(0, std::numeric_limits<uint32_t>::max()));

  interest->setInterestLifetime(ndn::time::seconds(3));

  packet_count++;

  NS_LOG_DEBUG("Sending Interest packet for " << *interest);

  // Call trace (for logging purposes)
  m_transmittedInterests(interest, this, m_face);

  m_appLink->onReceiveInterest(*interest);
}

// Callback that will be called when Interest arrives
void
DataStore::OnInterest(std::shared_ptr<const ndn::Interest> interest)
{
  ndn::App::OnInterest(interest);

  NS_LOG_DEBUG("Received Record for " << interest->getName());

  std::string inputString = interest->getName().toUri();

    int head = 0, tail;
    std::string DataName, TargetNode, itemType, SourceNode;
    
    for (int i = 0; i <= 6; i++)
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
        break;
      case 4:
        SourceNode = temp;
        //NS_LOG_DEBUG("targetnode = " << TargetNode);
        break;
      case 5:
        DataName = temp;
        //NS_LOG_DEBUG("dataname = " << DataName);
        break;
      case 6:
        itemType = temp;
        //NS_LOG_DEBUG("dataname = " << DataName);
        break;
      }
    }

  if (itemType == "Store_complete")
  {
    // GetK_ptr()->Delete_data(DataName);
    std::cout<< GetK_ptr()->GetKId() << "  data Delete: " << DataName <<"\n";
    return;
  }

  if (itemType == "Transform_Data_Duplicate")
  {
    GetK_ptr()->SetData(DataName, GetK_ptr()->GetKId() , SourceNode, true);
    return;
  }

  //上下線狀態判定
  if (!GetK_ptr()->GetisOnline())
  {
    NS_LOG_DEBUG("error! this node is offline : " << interest->getName());
    if (itemType == "Transform_Data")
    {
      GetK_ptr()->SetData(DataName, GetK_ptr()->GetKId() , SourceNode, false);
    }
    return;
  }
  
  std::size_t hashRecord = std::hash<std::string>{}(DataName);
  std::string binaryRecord = std::bitset<16>(hashRecord).to_string();
  std::string nextTarget = GetK_ptr()->GetNext_Node(binaryRecord, 1, SourceNode);

  if (GetK_ptr()->GetKId() == nextTarget)
  {

    if (TargetNode != SourceNode)
    {
      GetK_ptr()->SetData(DataName, GetK_ptr()->GetKId() , SourceNode, false);
    }
    //GetK_ptr()->Node_info();

    std::string module_choose = "download";

    //若已完成Transform，則不用回傳儲存完畢訊息
    if (itemType == "Transform_Data" || itemType == "Transform_Data_Duplicate")
    {
      return;
    }

    //選擇另外兩個節點進行資料複製
    std::pair<std::string, std::string> transform_node;
    transform_node.first = "NULL";
    transform_node.second = "NULL";

    for (int i = 4; i <= 12; i = i+4)
    {
       std::string* K_bucket = GetK_ptr()->GetK_bucket(i);
       for (int j = 0; j < GetK_ptr()->GetK_bucket_size() ; j++)
       {
         std::string tempNode = K_bucket[j];
         if (tempNode == "NULL" || tempNode == GetK_ptr()->GetKId())
         {
           continue;
         }

         if (transform_node.first == "NULL")
         {
           transform_node.first = tempNode;
           continue;
         }

         if (transform_node.second == "NULL")
         {
           transform_node.second = tempNode;
           if (GetK_ptr()->XOR(transform_node.first, binaryRecord) > GetK_ptr()->XOR(transform_node.second, binaryRecord))
          {
            tempNode = transform_node.first;
            transform_node.first = transform_node.second;
            transform_node.second = tempNode;
          }
           continue;
         }
         
         if (GetK_ptr()->XOR(transform_node.first, binaryRecord) < GetK_ptr()->XOR(tempNode, binaryRecord))
         {
           transform_node.first = tempNode;
         }

         if (GetK_ptr()->XOR(transform_node.first, binaryRecord) > GetK_ptr()->XOR(transform_node.second, binaryRecord))
         {
           tempNode = transform_node.first;
           transform_node.first = transform_node.second;
           transform_node.second = tempNode;
         }
         
       }
       
    }

    //傳送備份給另外兩個節點
    if (transform_node.first != "NULL")
    {
      ndn::Name first_trans;
      first_trans.append("prefix").append("data").append("store").append(transform_node.first).append(GetK_ptr()->GetKId()).append(DataName).append("Transform_Data_Duplicate");
      auto first_interest = std::make_shared<ndn::Interest>(first_trans);
      Ptr<UniformRandomVariable> rand = CreateObject<UniformRandomVariable>();
      first_interest->setNonce(rand->GetValue(0, std::numeric_limits<uint32_t>::max()));
      first_interest->setMustBeFresh(1);
      first_interest->setInterestLifetime(ndn::time::seconds(3));
      m_transmittedInterests(first_interest, this, m_face);
      
      m_appLink->onReceiveInterest(*first_interest);

    }
    else
    {
      std::cout << "error: no other node to Duplicate data\n";
    }
    
    
    if (transform_node.second != "NULL")
    {
      ndn::Name second_trans;
      second_trans.append("prefix").append("data").append("store").append(transform_node.second).append(GetK_ptr()->GetKId()).append(DataName).append("Transform_Data_Duplicate");
      auto second_interest = std::make_shared<ndn::Interest>(second_trans);
      Ptr<UniformRandomVariable> rand = CreateObject<UniformRandomVariable>();
      second_interest->setNonce(rand->GetValue(0, std::numeric_limits<uint32_t>::max()));
      second_interest->setMustBeFresh(1);
      second_interest->setInterestLifetime(ndn::time::seconds(3));
      m_transmittedInterests(second_interest, this, m_face);

      m_appLink->onReceiveInterest(*second_interest);
    }
    else
    {
      std::cout << "error: no other node to Duplicate data\n";
    } 
    

    //回傳給源節點，進行dataSet Update
    ndn::Name next ;
    next.append("prefix").append("data").append(module_choose).append(SourceNode).append("NULL").append(DataName).append("Store_complete");
    auto next_interest = std::make_shared<ndn::Interest>(next);
    Ptr<UniformRandomVariable> rand = CreateObject<UniformRandomVariable>();
    next_interest->setNonce(rand->GetValue(0, std::numeric_limits<uint32_t>::max()));
    next_interest->setMustBeFresh(1);
    next_interest->setInterestLifetime(ndn::time::seconds(3));


    NS_LOG_DEBUG("Store-data-complete " << DataName);

  
    m_transmittedInterests(next_interest, this, m_face);

    m_appLink->onReceiveInterest(*next_interest);
  }
  else
  {
    ndn::Name next ;
    
    next.append("prefix").append("data").append("store").append(nextTarget).append(SourceNode).append(DataName).append(itemType);
    auto next_interest = std::make_shared<ndn::Interest>(next);
    Ptr<UniformRandomVariable> rand = CreateObject<UniformRandomVariable>();
    next_interest->setNonce(rand->GetValue(0, std::numeric_limits<uint32_t>::max()));
    next_interest->setMustBeFresh(1);
    next_interest->setInterestLifetime(ndn::time::seconds(3));


    NS_LOG_DEBUG("Sending Interest packet to another node : " << *next_interest);

  
    m_transmittedInterests(next_interest, this, m_face);

    m_appLink->onReceiveInterest(*next_interest);
  }

}

// Callback that will be called when Data arrives
void
DataStore::OnData(std::shared_ptr<const ndn::Data> data)
{
  NS_LOG_DEBUG("Receiving Data packet for " << data->getName());

  std::cout << "DATA received for name " << data->getName() << std::endl;
}

void
DataStore::SetNode_Pointer(Ptr<Node> input)
{
  parent_node = input;
}

} // namespace ns3

