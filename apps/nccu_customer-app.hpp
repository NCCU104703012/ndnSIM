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
  void
  SendRecord();

  void
  SetRecord(std::string input);

  void
  SetNode_Pointer(Ptr<Node> input);

  void
  SendQuery();


private:
  void
  SendInterest();

  ndn::Name m_prefix;
  ndn::Name m_postfix;
  uint32_t m_virtualPayloadSize;
  Time m_freshness;

  uint32_t m_signature;
  ndn::Name m_keyLocator;

  int packet_count = 0;
  Ptr<Node> parent_node ;

  //本身節點的交易紀錄 用來送出給Data store
  std::string record;

  //用來製作興趣封包
  ndn::Name NodeName;

  //送出Query使用
  ndn::Name Query;
  //[5] = {"data8", "initRecord0", "initRecord1", "initRecord1", "trash"};
  int query_count = 1;

};

} // namespace ns3

#endif // CUSTOMER_APP_H_
