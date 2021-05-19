
#include <iostream>
#include <string>
#include "kademlia.hpp"

Kademlia::Kademlia(std::string node_name_input, std::string data_input, int kademliaID)
{
    node_name = node_name_input;
    dataList = new Data();
    dataList->Name = data_input;
    KId = kademliaID;
    
}

bool
Kademlia::GetData(std::string input){
    //return data.compare(input);
}

Kademlia *
Kademlia::GetNext_Node(std::string TargetNode){
    int distance = KId - std::stoi(TargetNode);
    distance = abs(distance);
    Kademlia *output = this;
   for (int i = 0; i < knum; i++)
   {
       int temp = k_bucket[i]->KId;
       if(abs(temp - std::stoi(TargetNode)) < distance)
       {
           output = k_bucket[i];
       }
       
   }
   return output;
}


void
Kademlia::Node_info(){
    std::cout << "**************" << std::endl;
    // std::cout << "node name = " << node_name << std::endl;
    // std::cout << "data = " << data << std::endl;
    // std::cout << "next node = " << next_node << std::endl;
    dataList->printAllData();
    std::cout << std::endl << "**************" << std::endl;
}

void
Kademlia::Set_KBucket(Kademlia *KNode)
{
    k_bucket[knum] = KNode;
    knum++;
}

void
Kademlia::SetData(std::string input)
{
    dataList->AddData(input);
}

//---------------------Class Data-------------------------

void
Data::AddData(std::string inputName)
{
    Data *inputData = new Data();
    Data *tailPtr = this->GetTail();
    inputData->head = tailPtr->head;
    tailPtr->next = inputData;
    inputData->Name = inputName;
}

void
Data::printAllData()
{
    std::cout << Name << "||";
    Data *tempPtr = this->next;
    while (tempPtr != NULL)
    {
        std::cout << tempPtr->Name << "||";
        tempPtr = tempPtr->next;
    }
    
}

Data*
Data::GetTail()
{
    Data *outputPtr = this;
    Data *tempPtr = this->next;
    while (tempPtr != NULL)
    {
        outputPtr = tempPtr;
        tempPtr = tempPtr->next;
    }
    return outputPtr;
}