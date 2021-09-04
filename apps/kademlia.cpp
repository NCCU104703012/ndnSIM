
#include <iostream>
#include <string>
#include <bitset>
#include "kademlia.hpp"

Kademlia::Kademlia(std::string node_name_input, std::string data_input, std::string kademliaID)
{
    node_name = node_name_input;
    dataList = new Data();
    dataList->head = dataList;
    dataList->Name = data_input;
    queryList = new Data();
    queryList->head = queryList;
    queryList->Name = "init";
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

std::string
Kademlia::GetNext_Node(std::string TargetNode, int output_num){

    std::string output[3] = {this->KId, "NULL", "NULL"};
    std::string outputString = "";
    int arrIndex = 0;

    for (int i = 0; i < 15; i++)
    {
        if (k_bucket[i] != "NULL")
        {
            if (output[arrIndex%3] == "NULL")
            {
                output[arrIndex%3] = k_bucket[i];
                arrIndex++;
            }
            else if (this->XOR(TargetNode, k_bucket[i]) < this->XOR(TargetNode, output[arrIndex%3]))
            {
                output[arrIndex%3] = k_bucket[i];
                arrIndex++;
            }
        } 
    }

    if (output_num == 1)
    {
        return output[(arrIndex-1)%3];
    }

    for (int i = 0; i < 3; i++)
    {
        if (output[i] != "NULL")
        {
            outputString = outputString + "_" + output[i];
        }
        
    }
    
    //未完成，需要return output_num數量的Knode資訊
    return outputString;
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

int
Kademlia::XOR(std::string input, std::string input2)
{
    int distance = 0;
    for (int i = 1; i < 9; i++)
    {
        std::string str1 = input2.substr(i,1);
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
    //std::cout << "**************" << std::endl;
    // std::cout << "node name = " << node_name << std::endl;
    // std::cout << "data = " << data << std::endl;
    // std::cout << "next node = " << next_node << std::endl;
    dataList->printAllData();
    //std::cout << std::endl << "**************" << std::endl;
}

void
Kademlia::Set_KBucket(std::string KNode)
{
    for (int i = 0; i < 15; i++)
    {
        if (k_bucket[i] == "NULL")
        {
            k_bucket[i] = KNode;
            return;
        }
    }
}

void
Kademlia::SetData(std::string input, std::string type)
{
    dataList->AddData(input, type);
}

//以輸入的節點名稱比較其他K桶中資訊，並決定是否將其加入
std::string
Kademlia::KBucket_update(std::string sourceNode)
{
    if (sourceNode == KId)
    {
        return "NULL";
    }
    

    for (int i = 0; i < 15; i++)
    {
        if (k_bucket[i] == sourceNode)
        {
            //std::cout << "already have same node in K-buk\n";
            return "NULL";
        }    
    }

    for (int i = 0; i < 15; i++)
    {
        if (k_bucket[i] == "NULL")
        {
            k_bucket[i] = sourceNode;
            return "add sourceNode to a NULL";
        }
    }
    

    //演算法需要決策出一個替換的節點
    return "K-buk is full";
}

//將輸入節點從k桶中去除
void
Kademlia::KBucket_delete(std::string sourceNode)
{
    for (int i = 0; i < 15; i++)
    {
        if (k_bucket[i] == sourceNode)
        {
            k_bucket[i] = "NULL";
            return;
        }
        
    }
    
}

//從datalist中刪除資料
void
Kademlia::Delete_data(std::string DataName)
{
    Data* targetPtr = dataList->GetData(DataName);

    if (targetPtr == NULL)
    {
        std::cout << "In delete_data(), GetData() is NULL\n";
        return;
    }
    
    Data* prePtr = dataList;

    while (1)
    {
        if (prePtr->next == targetPtr)
        {
            prePtr->next = targetPtr->next;
            delete targetPtr;
            return;
        }
        prePtr = prePtr->next;

        if (prePtr == NULL)
        {
           return;
        }
        
    }
    
}

//從datalist中刪除資料
void
Kademlia::Delete_data_query(std::string DataName)
{
    Data* targetPtr = queryList->GetData(DataName);

    if (targetPtr == NULL)
    {
        std::cout << "In delete_data(), GetData() is NULL\n";
        return;
    }
    
    Data* prePtr = dataList;

    while (1)
    {
        if (prePtr->next == targetPtr)
        {
            prePtr->next = targetPtr->next;
            delete targetPtr;
            return;
        }
        prePtr = prePtr->next;

        if (prePtr == NULL)
        {
           return;
        }
        
    }
    
}

//針對輸入節點，比較所有更接近的資料，回傳整理的字串
std::string
Kademlia::Transform_Data(std::string thisNode, std::string targetNode)
{

    std::string output = "|";
    Data* DataPtr = dataList->next;

    while (DataPtr != NULL)
    {
        std::size_t hashData = std::hash<std::string>{}(DataPtr->Name);
        std::string binaryData = std::bitset<8>(hashData).to_string();
        int thisNode_distance = XOR(thisNode, binaryData);
        int targetNode_distance = XOR(targetNode, binaryData);

        if (targetNode_distance < thisNode_distance)
        {
            output = output + DataPtr->Name + "|" ;
        }
        DataPtr = DataPtr->next;
    }
    return output;
}



//---------------------Class Data-------------------------

void
Data::AddData(std::string inputName, std::string inputType)
{
    std::cout << "add data: " << inputName << "\n";
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

void
Data::update_nextHop(std::string inputNode)
{
    int distance;
    std::size_t hashData = std::hash<std::string>{}(this->Name);
    std::string binaryData = std::bitset<8>(hashData).to_string();
    distance = XOR(binaryData,inputNode);

    if (distance >= XOR(binaryData, closest_node))
    {
        return;
    }

    for (int  i = 0; i < 3; i++)
    {
        if (this->nextHop_list[i] == "NULL")
        {
            this->nextHop_list[i] = inputNode;
            return;
        }
    }
    
    for (int i = 0; i < 3; i++)
    {
        if (this->nextHop_list[i] == inputNode)
        {
            // 預防nextHop list出現相同節點，重複Query
            return;
        }
        if (distance < XOR(binaryData, this->nextHop_list[i]))
        {
            this->nextHop_list[i] = inputNode;
            return;
        }     
    }
    
}

int
Data::XOR(std::string input, std::string input2)
{
    int distance = 0;
    for (int i = 1; i < 9; i++)
    {
        std::string str1 = input2.substr(i,1);
        std::string str2 = input.substr(i,1);
        if (str1.compare(str2))
        {
            distance++;
        }
        
    }
    return distance;
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
Order::AddOrder(std::string orderName, std::string dataString, double timeString, int targetNumber)
{
    Order* inputData = new Order(orderName, dataString, timeString, targetNumber);
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

Order*
Order::AddOrder_toTail(std::string orderName, std::string dataString, double timeString, int targetNumber)
{
    Order* inputData = new Order(orderName, dataString, timeString, targetNumber);
    Order* tempPtr = this;
    while (tempPtr->getNext() != NULL)
    {
        tempPtr = tempPtr->getNext();
    }
    tempPtr->setNext(inputData);
    return inputData;
}

//搜尋此order有無資料，有則回傳true，並修改dataList，fulfillList
bool
Order::checkDataList(std::string DataName)
{
    std::set<std::string> tempList = this->getDataList();
    if (tempList.find(DataName) != tempList.end())
    {
        tempList.erase(DataName);
        this->setFulfill_List(DataName);
        this->replace_Datalist(tempList);
        return true;
    }
    return false;
    
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

void
Order::deleteOrder(Order* preOrder)
{
    preOrder->setNext(nextPtr);
    delete this;
}

