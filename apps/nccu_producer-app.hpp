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

// hijacker.hpp

#ifndef PERDUCER_H_
#define PERDUCER_H_

#include "ns3/ndnSIM/apps/ndn-app.hpp"

namespace ns3 {

class ProudcerApp : public ndn::App {
public:
  static TypeId
  GetTypeId();

  ProudcerApp();

  // Receive all Interests but do nothing in response
  void
  OnInterest(std::shared_ptr<const ndn::Interest> interest);

protected:
  // inherited from Application base class.
  virtual void
  StartApplication();

  virtual void
  StopApplication();
};

} // namespace ns3

#endif // PERDUCER_H_
