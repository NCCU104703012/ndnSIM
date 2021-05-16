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

// hijacker.cpp
#include <iostream>
#include <string> 
#include<unistd.h>

#include "nccu_producer-app.hpp"

#include "ns3/log.h"
#include "ns3/string.h"
#include "ns3/uinteger.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"

#include "model/ndn-l3-protocol.hpp"
#include "ns3/ndnSIM/helper/ndn-fib-helper.hpp"

#include <memory>
#include <string>

NS_LOG_COMPONENT_DEFINE("ProducerApp");

namespace ns3 {
namespace ndn {

// Necessary if you are planning to use ndn::AppHelper
NS_OBJECT_ENSURE_REGISTERED(ProducerApp);

TypeId
ProducerApp::GetTypeId()
{
  static TypeId tid = 
    TypeId("ns3::ndn::ProducerApp")
      .SetGroupName("Ndn")
      .SetParent<App>()
      .AddConstructor<ProducerApp>()
      .AddAttribute("Prefix", "Prefix, for which producer has the data", StringValue("/"),
                    MakeNameAccessor(&ProducerApp::m_prefix), MakeNameChecker())
      .AddAttribute(
         "Postfix",
         "Postfix that is added to the output data (e.g., for adding producer-uniqueness)",
         StringValue("/"), MakeNameAccessor(&ProducerApp::m_postfix), MakeNameChecker())
      .AddAttribute("PayloadSize", "Virtual payload size for Content packets", UintegerValue(1024),
                    MakeUintegerAccessor(&ProducerApp::m_virtualPayloadSize),
                    MakeUintegerChecker<uint32_t>())
      .AddAttribute("Freshness", "Freshness of data packets, if 0, then unlimited freshness",
                    TimeValue(Seconds(0)), MakeTimeAccessor(&ProducerApp::m_freshness),
                    MakeTimeChecker())
      .AddAttribute(
         "Signature",
         "Fake signature, 0 valid signature (default), other values application-specific",
         UintegerValue(0), MakeUintegerAccessor(&ProducerApp::m_signature),
         MakeUintegerChecker<uint32_t>())
      .AddAttribute("KeyLocator",
                    "Name to be used for key locator.  If root, then key locator is not used",
                    NameValue(), MakeNameAccessor(&ProducerApp::m_keyLocator), MakeNameChecker());
  return tid;
}

ProducerApp::ProducerApp()
{
  NS_LOG_FUNCTION_NOARGS();
}

void
ProducerApp::StartApplication()
{
  NS_LOG_FUNCTION_NOARGS();
  App::StartApplication();

  // equivalent to setting interest filter for "/prefix" prefix
  // ndn::FibHelper::AddRoute(GetNode(), "/prefix/food/1", m_face, 0);
  FibHelper::AddRoute(GetNode(), m_prefix, m_face, 0);
}

void
ProducerApp::StopApplication()
{
  NS_LOG_FUNCTION_NOARGS();

  App::StopApplication();
}

void
ProducerApp::OnInterest(std::shared_ptr<const ndn::Interest> interest)
{
  ndn::App::OnInterest(interest); // forward call to perform app-level tracing
  // do nothing else (hijack interest)

  NS_LOG_FUNCTION(this << interest);

  if (!m_active)
    return;

  // nccu:偵錯LOG:負責處理回傳資料
  Name dataName(interest->getName());
  // dataName.append(m_postfix);
  // dataName.appendVersion();
  //Name dataName("/prefix/food");

  std::cout << "------dataName test--------" << std::endl;
  std::string temp = dataName.toUri();
  std::cout << temp <<std::endl;
  std::cout << "------dataName test--------" << std::endl;

  auto data = make_shared<Data>();
  data->setName(dataName);
  data->setFreshnessPeriod(::ndn::time::milliseconds(m_freshness.GetMilliSeconds()));

  data->setContent(make_shared< ::ndn::Buffer>(m_virtualPayloadSize));

  Signature signature;
  SignatureInfo signatureInfo(static_cast< ::ndn::tlv::SignatureTypeValue>(255));

  if (m_keyLocator.size() > 0) {
    signatureInfo.setKeyLocator(m_keyLocator);
  }

  signature.setInfo(signatureInfo);
  signature.setValue(::ndn::makeNonNegativeIntegerBlock(::ndn::tlv::SignatureValue, m_signature));

  data->setSignature(signature);

  // 確認回傳封包資訊
  NS_LOG_INFO("node(" << GetNode()->GetId() << ") responding with Data: " << data->getName() << "回傳資料封包號碼： " << std::to_string(check));
  check++;
  

  //std::cout << "data packet name : " << dataName << std::endl ;

  // to create real wire encoding
  data->wireEncode();

  m_transmittedDatas(data, this, m_face);
  m_appLink->onReceiveData(*data);

}



} // namespace ndn
} // namespace ns3
