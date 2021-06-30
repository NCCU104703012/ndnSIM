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
  Simulator::Schedule(Seconds(1.0), &CustomerApp::SendRecord, this);
  Simulator::Schedule(Seconds(1.5), &CustomerApp::SendRecord, this);
  Simulator::Schedule(Seconds(2.0), &CustomerApp::SendRecord, this);
  Simulator::Schedule(Seconds(2.5), &CustomerApp::SendRecord, this);
  Simulator::Schedule(Seconds(3.0), &CustomerApp::SendRecord, this);

  // Simulator::Schedule(Seconds(4), &CustomerApp::SendQuery, this);
  // Simulator::Schedule(Seconds(7), &CustomerApp::SendQuery, this);
  // Simulator::Schedule(Seconds(10), &CustomerApp::SendQuery, this);
  // Simulator::Schedule(Seconds(13), &CustomerApp::SendQuery, this);
  // Simulator::Schedule(Seconds(16), &CustomerApp::SendQuery, this);

  std::string timeList = GetO_ptr()->getTimeStamp();
  int head = 0, tail;
  for (int i = 0; i < GetO_ptr()->getTargetNum() ; i++)
  {
    tail = timeList.find_first_of("/", head);

    //std::cout << timeList.substr(head, tail-head) << std::endl;

    double temp = std::stod(timeList.substr(head, tail-head));
    Simulator::Schedule(Seconds(temp), &CustomerApp::SendQuery, this);
    head = tail+1;
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
    std::string DataName, TargetNode, SourceNode, flag;
    
    for (int i = 0; i < 6; i++)
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
      }
    }

  ndn::Name InsName;
  InsName.append("prefix").append("data").append("query").append(SourceNode).append("1").append(TargetNode).append(DataName);

  auto outinterest = std::make_shared<ndn::Interest>(InsName);
  Ptr<UniformRandomVariable> rand = CreateObject<UniformRandomVariable>();
  outinterest->setNonce(rand->GetValue(0, std::numeric_limits<uint32_t>::max()));

  outinterest->setInterestLifetime(ndn::time::seconds(1));

  NS_LOG_DEBUG("Sending Interest packet for " << *outinterest);

  // Call trace (for logging purposes)
  m_transmittedInterests(outinterest, this, m_face);

  m_appLink->onReceiveInterest(*outinterest);

}

// Callback that will be called when Data arrives
void
CustomerApp::OnData(std::shared_ptr<const ndn::Data> data)
{
  NS_LOG_DEBUG("Receiving Data packet for " << data->getName());

  std::cout << "DATA received for name " << data->getName() << std::endl;
}

void
CustomerApp::SetNode_Pointer(Ptr<Node> input)
{
  parent_node = input;
}



//送出一筆交易紀錄
void
CustomerApp::SendRecord()
{
  //record分割
  std::cout << Record << std::endl;

  int head = 0 , tail = Record.toUri().find_first_of("/");
  std::string record_output;
  for (int i = 0; i <= record_count; i++)
  {
    head = tail+1;
    tail = Record.toUri().find_first_of("/",head);
  }
  
  record_output = Record.toUri().substr(head, tail-head);

  std::size_t hashRecord = std::hash<std::string>{}(record_output);
  std::string binaryRecord = std::bitset<8>(hashRecord).to_string();
  NS_LOG_DEBUG("hash Record for " << binaryRecord << " " << record_output);

  ndn::Name temp;
  temp.append("prefix").append("data").append("store").append(NodeName);
  temp.append(GetK_ptr()->GetNext_Node(binaryRecord)->GetKId()).append(record_output);
  auto interest = std::make_shared<ndn::Interest>(temp);
  record_count++;

  Ptr<UniformRandomVariable> rand = CreateObject<UniformRandomVariable>();
  interest->setNonce(rand->GetValue(0, std::numeric_limits<uint32_t>::max()));

  interest->setInterestLifetime(ndn::time::seconds(3));
  NS_LOG_DEBUG("Sending Record for " << interest->getName());

  // Call trace (for logging purposes)
  m_transmittedInterests(interest, this, m_face);

  m_appLink->onReceiveInterest(*interest);
}


void
CustomerApp::SendQuery(){

  //query分割
  std::string orderName = GetO_ptr()->getDatalist();

  int head = 0 , tail = orderName.find_first_of("/");
  std::string query_output;
  for (int i = 0; i < query_count; i++)
  {
    head = tail+1;
    tail = orderName.find_first_of("/",head);
  }
  
  query_output = orderName.substr(head, tail-head);
  

  ndn::Name temp;
  temp.append("prefix").append("data").append("query").append(NodeName).append("0").append(NodeName);
  temp.append(query_output);
  query_count++;

  auto interest = std::make_shared<ndn::Interest>(temp);
  
  Ptr<UniformRandomVariable> rand = CreateObject<UniformRandomVariable>();
  interest->setNonce(rand->GetValue(0, std::numeric_limits<uint32_t>::max()));

  interest->setInterestLifetime(ndn::time::seconds(1));
  NS_LOG_DEBUG("Sending Query for " << interest->getName());

  // Call trace (for logging purposes)
  m_transmittedInterests(interest, this, m_face);

  m_appLink->onReceiveInterest(*interest);

}

} // namespace ns3
