
#include <iostream>
#include <string>
#include "kademlia.hpp"

Kademlia::Kademlia(std::string node_name_input, std::string data_input, std::string kademliaID)
{
    node_name = node_name_input;
    dataList = new Data();
    dataList->head = dataList;
    dataList->Name = data_input;
    KId = kademliaID;
    
}

bool
Kademlia::GetData(std::string DataName){
    Data *output = NULL;
    output = dataList->GetData(DataName);
    if (output == NULL)
    {
        return false;
    }
    else
    {
        return true;
    }
}

Kademlia *
Kademlia::GetNext_Node(std::string TargetNode){
    int distance = this->XOR(TargetNode);
    //std::cout << "distance is : " << distance << std::endl;
    Kademlia *output = this;

    for (int i = 0; i < knum; i++)
    {
        std::string temp = k_bucket[i]->KId;
        // std::cout << "input is : " << TargetNode << std::endl;
        // std::cout << "kbucket is : " << temp << std::endl;
        if (k_bucket[i]->XOR(TargetNode) < distance)
        {
            output = k_bucket[i];
        }
        
    }
    

//     int distance = KId - std::stoi(TargetNode);
//     distance = abs(distance);
//     Kademlia *output = this;
//    for (int i = 0; i < knum; i++)
//    {
//        int temp = k_bucket[i]->KId;
//        if(abs(temp - std::stoi(TargetNode)) < distance)
//        {
//            output = k_bucket[i];
//        }
       
//    }
    return output;
}

int
Kademlia::XOR(std::string input)
{
    int distance = 0;
    for (int i = 1; i < 9; i++)
    {
        std::string str1 = this->KId.substr(i,1);
        std::string str2 = input.substr(i,1);
        if (str1.compare(str2))
        {
            distance++;
        }
        
    }
    return distance;
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
Kademlia::SetData(std::string input, std::string type)
{
    dataList->AddData(input, type);
}

//---------------------Class Data-------------------------

void
Data::AddData(std::string inputName, std::string inputType)
{
    Data *inputData = new Data();
    Data *tailPtr = this->GetTail();
    inputData->head = tailPtr->head;
    tailPtr->next = inputData;
    inputData->Name = inputName;
    inputData->type = inputType;
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

Data*
Data::GetData(std::string DataName)
{
    Data *tempptr = this;
    while (tempptr != NULL)
    {
        if (DataName.compare(tempptr->Name) == 0)
        {
            return tempptr;
        }
        else
        {
            tempptr = tempptr->next;
        }
    }
    return NULL;
}

//---------------------Class Order-------------------------

void Order::setNext(Order* ptr){
    nextPtr = ptr;
}


Order* Order::getTail(){
    Order *outputPtr = this;
    Order *tempPtr = this->getNext();
    while (tempPtr != NULL)
    {
        outputPtr = tempPtr;
        tempPtr = tempPtr->getNext();
    }
    return outputPtr;
}

void
Order::AddOrder(std::string dataString, double timeString, int targetNumber)
{
    Order* inputData = new Order(dataString, timeString, targetNumber);
    Order* tempPtr = this;
    Order* prePtr = this;

    while (timeString > tempPtr->getTimeStamp() && tempPtr->getNext() != NULL)
    {
        prePtr = tempPtr;
        tempPtr = tempPtr->getNext();
    }
    if (tempPtr->getNext() == NULL && timeString > tempPtr->getTimeStamp())
    {
        tempPtr->setNext(inputData);
    }
    else
    {
        inputData->setNext(tempPtr);
        prePtr->setNext(inputData);
    }
}

//搜尋整個order資料結構 滿足所需資料的所有order
void
Order::checkDataList(std::string DataName, std::string itemtype)
{
    std::set<std::string> tempList = this->getDataList();
    if (tempList.find(itemtype + "/" + DataName) != tempList.end())
    {
        tempList.erase(itemtype + "/" + DataName);
        this->setFulfill_List(itemtype + "/" + DataName);
        this->replace_Datalist(tempList);
        //this->checkFulFill();
    }
    
}
    
//確認此筆order是否完成
bool  
Order::checkFulFill()
{
    if (static_cast<int>(this->getDataList().size()) == 0)
    {
    //    std::cout << "******************" << std::endl;
    //    std::cout << "This Order is done!!!" << std::endl;
    //    std::cout << "******************" << std::endl;
       return true;
    }
    else
    {
        return false;
    }
    
}

