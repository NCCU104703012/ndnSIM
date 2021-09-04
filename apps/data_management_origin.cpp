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

#include "data_management_origin.hpp"

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

NS_LOG_COMPONENT_DEFINE("DataManageOrigin");

namespace ns3 {

NS_OBJECT_ENSURE_REGISTERED(DataManageOrigin);

// register NS-3 type
TypeId
DataManageOrigin::GetTypeId()
{
  static TypeId tid = 
    TypeId("DataManageOrigin")
      .SetGroupName("Ndn")
      .SetParent<App>()
      .AddConstructor<DataManageOrigin>()
      .AddAttribute("Prefix", "Prefix, for which producer has the data", StringValue("/"),
                    MakeNameAccessor(&DataManageOrigin::m_prefix), ndn::MakeNameChecker())
      .AddAttribute(
         "Postfix",
         "Postfix that is added to the output data (e.g., for adding producer-uniqueness)",
         StringValue("/"), MakeNameAccessor(&DataManageOrigin::m_postfix), ndn::MakeNameChecker())
      .AddAttribute("PayloadSize", "Virtual payload size for Content packets", UintegerValue(1024),
                    MakeUintegerAccessor(&DataManageOrigin::m_virtualPayloadSize),
                    MakeUintegerChecker<uint32_t>())
      .AddAttribute("Freshness", "Freshness of data packets, if 0, then unlimited freshness",
                    TimeValue(Seconds(0)), MakeTimeAccessor(&DataManageOrigin::m_freshness),
                    MakeTimeChecker())
      .AddAttribute(
         "Signature",
         "Fake signature, 0 valid signature (default), other values application-specific",
         UintegerValue(0), MakeUintegerAccessor(&DataManageOrigin::m_signature),
         MakeUintegerChecker<uint32_t>())
      .AddAttribute("KeyLocator",
                    "Name to be used for key locator.  If root, then key locator is not used",
                    ndn::NameValue(), MakeNameAccessor(&DataManageOrigin::m_keyLocator), ndn::MakeNameChecker())
      .AddAttribute("Query", "string of query", StringValue(""),
          MakeNameAccessor(&DataManageOrigin::Query), ndn::MakeNameChecker())
      .AddAttribute(
        "Kademlia",
        "Kademlia struct",
        StringValue(""),
        MakeNameAccessor(&DataManageOrigin::k_ptr),
        ndn::MakeNameChecker())
      ;

  return tid;
}

// Processing upon start of the application
void
DataManageOrigin::StartApplication()
{
  // initialize ndn::App
  ndn::App::StartApplication();

  // Add entry to FIB for `/prefix/sub`
  //ndn::FibHelper::AddRoute(GetNode(), prefix, m_face, 0);
  ndn::FibHelper::AddRoute(GetNode(), m_prefix, m_face, 0);
  //ndn::FibHelper::AddRoute(GetNode(), "/prefix/clothes", m_face, 0);

  // Schedule send of first interest
  for (int i = 0; i < 100; i++)
  {
    //設定開始進行上下線的時間, 以及一週的週期
    double startTime = 0.2;
    Simulator::Schedule(Seconds(startTime + 0.2*i), &DataManageOrigin::timeOut, this);
  }
}

// Processing when application is stopped
void
DataManageOrigin::StopApplication()
{
  // cleanup ndn::App
  ndn::App::StopApplication();
}

void
DataManageOrigin::SendInterest()
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
DataManageOrigin::OnInterest(std::shared_ptr<const ndn::Interest> interest)
{
  ndn::App::OnInterest(interest);

  NS_LOG_DEBUG("Received Interest packet for " << interest->getName());

  std::string inputString = interest->getName().toUri();

    int head = 0, tail;
    std::string DataName, TargetNode, SourceNode, flag, itemType, nextHop;
    
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
      case 8:
        nextHop = temp;
      }
    }

    ndn::Name outInterest;

    if (flag == "-1")
    {
        //告知dataManagement 目標節點已找到資料，可將待決資料結構刪除
        GetK_ptr()->Delete_data_query(DataName);
        return;
    }


    if (itemType == "init")
    {
        if (GetK_ptr()->GetData(DataName))
        {
            outInterest.append("prefix").append("data").append("download").append(SourceNode).append(TargetNode).append(DataName).append(itemType);
            SendInterest(outInterest, "getData return to customer: ", true);
            GetK_ptr()->Delete_data_query(queryDataPtr->Name);
            return;
        }

        //GetK_ptr()->queryList->AddData(DataName, "food");
        Data* queryDataPtr = GetK_ptr()->GetQueryItem(DataName);

        //特別注意！！！！會有queryData不見的狀況，目前出現率不高
        if (queryDataPtr == NULL || queryDataPtr == GetK_ptr()->queryList)
        {
            std::cout << "error: this queryData is deleted\n";
            return;
        }

        queryDataPtr->closest_node = SourceNode;

        for (int i = 0; i < 15; i++)
        {
            if (GetK_ptr()->GetK_bucket()[i] != "NULL")
            {
                queryDataPtr->update_nextHop(GetK_ptr()->GetK_bucket()[i]);
            }
        
        }

        std::cout << "nextHop List: " ;
        for (int i = 0; i < 3; i++)
        {
            std::cout << queryDataPtr->nextHop_list[i] << "  "  ;
        }
        std::cout<< "\n";

        bool hasNextHop = false;
        for (int i = 0; i < 3; i++)
        {
            if (queryDataPtr->nextHop_list[i] != "NULL")
            {
                hasNextHop = true;
                //將此round的Query目標紀錄，timeout時若一樣，則直接作為Data Lost
                queryDataPtr->timeout_check[i] = queryDataPtr->nextHop_list[i];

                ndn::Name Interest;
                Interest.append("prefix").append("data").append("query").append(queryDataPtr->nextHop_list[i]).append("0").append(SourceNode).append(DataName).append("next-round").append("NULL");
                SendInterest(Interest, "next round Query: ", true);
            }   
        }

        if (!hasNextHop)
        {
            std::cout << "NO-match-Data-&-next-Node: " << queryDataPtr->Name <<"\n";
            GetK_ptr()->Delete_data_query(queryDataPtr->Name);
        }

        return;
    }
    
    
    //收到其他節點的資訊，可能是幾個next hop，可能是無更接近節點
    if (itemType == "Return")
    {
        //紀錄進queryList
        int head = 0, tail;
        Data* queryDataPtr = GetK_ptr()->GetQueryItem(DataName);
        queryDataPtr->reply_count++;
        head = nextHop.find_first_of("_", head);
        tail = nextHop.find_first_of("_", head+1);

        //error check
        if (queryDataPtr == NULL)
        {
            std::cout << "error: queryDataPtr is NULL\n";
            return;
        }

        while (tail != -1)
        {
            std::string newNode = nextHop.substr(head+1, tail-head-1);

            std::cout <<"nextHop : " << nextHop <<std::endl;

            queryDataPtr->update_nextHop(nextHop);


            head = tail;
            tail = nextHop.find_first_of("_", head+1);
        }
        
        queryDataPtr->closest_node = queryDataPtr->nextHop_list[0];

        if (queryDataPtr->reply_count == 3)
        {
            if (queryDataPtr->nextHop_list[0] == "NULL")
            {
                NS_LOG_DEBUG("NO-match-Data-&-next-Node");
                GetK_ptr()->Delete_data_query(DataName);
                return;
            }
            for (int i = 0; i < 3; i++)
            {
                if (queryDataPtr->nextHop_list[i] != "NULL")
                {
                   ndn::Name outInterest;
                   std::string targetNode = queryDataPtr->nextHop_list[i];
                   queryDataPtr->nextHop_list[i] = "NULL";
                   outInterest.append("prefix").append("data").append("query").append(targetNode).append("0").append(SourceNode).append(DataName).append(itemType).append("NULL");
                   SendInterest(outInterest, "next round Query: ", true);
                }
                
            }
            queryDataPtr->reply_count = 0;
        }
        

        //紀錄最靠近的節點，作為下一次選節點的判斷標準，在將list社為NULL
        //若均已返回，則執行下一個群體query

        return;
    }
    


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

    //此節點擁有資料，送回給sourceNode
    else if(GetK_ptr()->GetData(DataName))
    {
      outInterest.append("prefix").append("data").append("download").append(SourceNode).append(TargetNode).append(DataName).append(itemType);
      SendInterest(outInterest, "getData return to customer: ", true);

      ndn::Name outInterest_dataQuery;
      outInterest_dataQuery.append("prefix").append("data").append("query").append(SourceNode).append("-1").append(SourceNode).append(DataName).append(itemType).append("NULL");

      SendInterest(outInterest_dataQuery, "getData return to dataManage: ", true);
    
    }
    else
    {
      //此節點沒有資料，確認是否有K桶節點可返回
      std::size_t biTemp = std::hash<std::string>{}(DataName);
      std::string binaryDataName = std::bitset<8>(biTemp).to_string();
      NS_LOG_DEBUG("hash Record for " << binaryDataName << " " << DataName);

      if (GetK_ptr()->GetNext_Node(binaryDataName, 1) == GetK_ptr()->GetNodeName())
      {
        outInterest.append("prefix").append("data").append("query").append(SourceNode).append("0").append(SourceNode).append(DataName).append(itemType).append("NULL");
      }
      else
      {
        TargetNode = GetK_ptr()->GetNext_Node(binaryDataName, 3);
        outInterest.append("prefix").append("data").append("query").append(SourceNode).append("0").append(SourceNode).append(DataName).append(itemType).append(TargetNode);
      }
        SendInterest(outInterest, "sendBack to SourceNode: ", true);
    }

}

// Callback that will be called when Data arrives
void
DataManageOrigin::OnData(std::shared_ptr<const ndn::Data> data)
{
  NS_LOG_DEBUG("Receiving Data packet for " << data->getName());

  std::cout << "DATA received for name " << data->getName() << std::endl;
}

void
DataManageOrigin::SetNode_Pointer(Ptr<Node> input)
{
  parent_node = input;
}


void
DataManageOrigin::SendInterest(ndn::Name prefix, std::string logging, bool freshness){
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

//定期執行 增加每個QueryData的time clock，過期時執行next round Query
void
DataManageOrigin::timeOut()
{
    Data* queryDataPtr = GetK_ptr()->queryList->next;

    while (queryDataPtr != NULL)
    {
        //設定執行間隔
        queryDataPtr->lifeTime = queryDataPtr->lifeTime + 0.2;

        //超過timeout者，確認是否有next hop，沒有即lost，有則送出
        if (queryDataPtr->lifeTime >= 0.4)
        {
            
            bool hasNextHop = false;
            
            for (int i = 0; i < 3; i++)
            {
                if (queryDataPtr->nextHop_list[i] != "NULL")
                {
                    hasNextHop = true;
                    ndn::Name outInterest;
                    std::string targetNode = queryDataPtr->nextHop_list[i];
                    queryDataPtr->nextHop_list[i] = "NULL";
                    outInterest.append("prefix").append("data").append("query").append(targetNode).append("0").append(GetK_ptr()->GetKId()).append(queryDataPtr->Name).append("timeOut").append("NULL");
                    SendInterest(outInterest, " TimeOut next round Query: ", true);
                }
            }

            if (hasNextHop)
            {
                queryDataPtr->reply_count = 0;
                queryDataPtr->lifeTime = 0;
                std::cout << "Timeout! Go to Next round: " << queryDataPtr->Name << "\n";
            }
            else
            {
                std::cout << "NO-match-Data-&-next-Node: " << queryDataPtr->Name <<"\n";
                GetK_ptr()->Delete_data_query(queryDataPtr->Name);
            }
            
        }
        queryDataPtr = queryDataPtr->next;
    }
    
}


} // namespace ns3
