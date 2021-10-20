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

#ifndef DataManage_H_
#define DataManage_H_

#include "ns3/ndnSIM/apps/ndn-app.hpp"
#include "apps/kademlia.hpp"

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
class DataManage : public ndn::App {
public:
  // register NS-3 type "DataManage"
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

  //NDN錯誤容忍，輸出下一個目標前綴
  std::string
  ndnFault_tolerant(std::string DataName);

  // void
  // SetPrefix(std::string prefix);

  void
  SetNode_Pointer(Ptr<Node> input);

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

private:

  //此function暫未使用
  void
  SendInterest();

  ndn::Name m_prefix;
  ndn::Name m_postfix;
  uint32_t m_virtualPayloadSize;
  Time m_freshness;

  uint32_t m_signature;
  ndn::Name m_keyLocator;

  //k_ptr內容為"/0x00000" 不為純位址 要注意
  ndn::Name k_ptr;

  //送出Query使用
  ndn::Name Query;

  Ptr<Node> parent_node ;
};

} // namespace ns3

#endif // DataManage_H_
