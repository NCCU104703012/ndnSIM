
#include <iostream>
#include <string>
#include <bitset>
#include <math.h>
#include "kademlia.hpp"

#include <stdio.h>
#include <sqlite3.h>

int Kbucket_size = 10;

Kademlia::Kademlia(std::string node_name_input, std::string data_input, std::string kademliaID, sqlite3* inputDB)
{
    node_name = node_name_input;
    dataList = new Data();
    dataList->head = dataList;
    dataList->Name = data_input;
    queryList = new Data();
    queryList->head = queryList;
    queryList->Name = "init";
    KId = kademliaID;
    db = inputDB;
    dataList->db = inputDB;
    queryList->db = inputDB;
    
}

int
Kademlia::GetK_bucket_size(){return Kbucket_size;};

bool
Kademlia::GetData(std::string DataName){
    Data *output = NULL;
    output = dataList->GetData(DataName, GetKId());
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
Kademlia::GetNext_Node(std::string TargetNode, int output_num, std::string SourceNode){

    std::string output[3] = {this->KId, this->KId, this->KId};
    std::string outputString = "";
    std::pair<std::string, int> mostClose_Node;
    std::pair<std::string, int> mostFar_Node;
    // std::string mostClose_Node = this->KId;
    // std::string mostFar_Node = this->KId;
    // int arrIndex = 0;

    mostClose_Node.first = this->KId;
    mostClose_Node.second = 0;
    mostFar_Node.first = this->KId;
    mostFar_Node.second = 0;

    for (int i = 0; i < GetK_bucket_size(); i++)
    {
        if (SourceNode == k_bucket[i] || k_bucket[i] == "NULL")
        {
            continue;
        }

        if (this->XOR(k_bucket[i], TargetNode) > this->XOR(TargetNode, mostFar_Node.first))
        {
            output[mostFar_Node.second] = k_bucket[i];
            
            mostFar_Node.first = output[0];
            mostFar_Node.second = 0;    

            for (int j = 0; j < 3; j++)
            {
                if (this->XOR(output[j], TargetNode) > this->XOR(TargetNode, mostClose_Node.first))
                {
                    mostClose_Node.first = output[j];
                    mostClose_Node.second = i;
                }
                if (this->XOR(output[j], TargetNode) < this->XOR(TargetNode, mostFar_Node.first))
                {
                    mostFar_Node.first = output[j];
                    mostFar_Node.second = j;
                }
            }
        }
    }

    for (int i = 0; i < GetK_bucket_size(); i++)
    {
        if (SourceNode == k_bucket4[i] || k_bucket4[i] == "NULL")
        {
            continue;
        }

        if (this->XOR(k_bucket4[i], TargetNode) > this->XOR(TargetNode, mostFar_Node.first))
        {
            output[mostFar_Node.second] = k_bucket4[i];

            mostFar_Node.first = output[0];
            mostFar_Node.second = 0;    

            for (int j = 0; j < 3; j++)
            {
                if (this->XOR(output[j], TargetNode) > this->XOR(TargetNode, mostClose_Node.first))
                {
                    mostClose_Node.first = output[j];
                    mostClose_Node.second = i;
                }
                if (this->XOR(output[j], TargetNode) < this->XOR(TargetNode, mostFar_Node.first))
                {
                    mostFar_Node.first = output[j];
                    mostFar_Node.second = j;
                }
            }
        }
    }

    for (int i = 0; i < GetK_bucket_size(); i++)
    {
        if (SourceNode == k_bucket8[i] || k_bucket8[i] == "NULL")
        {
            continue;
        }

        if (this->XOR(k_bucket8[i], TargetNode) > this->XOR(TargetNode, mostFar_Node.first))
        {
            output[mostFar_Node.second] = k_bucket8[i];

            mostFar_Node.first = output[0];
            mostFar_Node.second = 0;    

            for (int j = 0; j < 3; j++)
            {
                if (this->XOR(output[j], TargetNode) > this->XOR(TargetNode, mostClose_Node.first))
                {
                    mostClose_Node.first = output[j];
                    mostClose_Node.second = i;
                }
                if (this->XOR(output[j], TargetNode) < this->XOR(TargetNode, mostFar_Node.first))
                {
                    mostFar_Node.first = output[j];
                    mostFar_Node.second = j;
                }
            }
        }
    }


    if (output_num == 1)
    {
        return mostClose_Node.first;
    }

    for (int i = 0; i < 3; i++)
    {
            outputString = outputString + "_" + output[i];
    }

    outputString = outputString + "_";
    
    //未完成，需要return output_num數量的Knode資訊
    return outputString;
}

int
Kademlia::GetSameBits(std::string inputKID){
    int distance = 0;
    for (int i = 0; i < 16; i++)
    {
        std::string str1 = std::to_string(this->KId[i]);
        std::string str2 = std::to_string(inputKID[i]);
        if (str1 == str2)
        {
            distance = distance + 1;
        }
        else
        {
            return distance;
        }
    }
    return distance;
}

long int
Kademlia::XOR(std::string input)
{
    // std::cout << "Xor: " << this->KId << " and " << input << "\n";

    long int distance = 0;
    for (int i = 0; i < 16; i++)
    {
        std::string str1 = std::to_string(this->KId[i]);
        std::string str2 = std::to_string(input[i]);
        if (str1 == str2)
        {
            distance = distance + pow(2, 16-i);
        }
        
    }
    return distance;
}

long int
Kademlia::XOR(std::string input, std::string input2)
{
    //std::cout << "Xor2: " << input2 << " and " << input << "\n";

    long int distance = 0;
    for (int i = 0; i < 16; i++)
    {
        std::string str1 = std::to_string(input2[i]);
        std::string str2 = std::to_string(input[i]);
        if (str1 == str2)
        {
            distance = distance + pow(2, 16-i);
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
    std::string* tempKbuk = GetK_bucket(GetSameBits(KNode));
    for (int i = 0; i < GetK_bucket_size(); i++)
    {
        if (tempKbuk[i] == "NULL")
        {
            tempKbuk[i] = KNode;
            return;
        }
    }
}

void
Kademlia::SetData(std::string input, std::string K_id, std::string sourceNode, bool init)
{
    dataList->AddData(input, K_id, sourceNode, init);
}

//以輸入的節點名稱比較其他K桶中資訊，並決定是否將其加入, distance指定選擇哪一個K桶
std::pair<std::string, std::string>
Kademlia::KBucket_update(std::string sourceNode , int distance)
{
    std::pair<std::string, std::string>outputPair("NULL", "NULL");
    std::string* tempKbuk = GetK_bucket(distance);

    if (sourceNode == KId)
    {
        return outputPair;
    }
    

    for (int i = 0; i < GetK_bucket_size(); i++)
    {
        if (tempKbuk[i] == sourceNode)
        {
            outputPair.first = sourceNode;
            return outputPair;
        }    
    }

    for (int i = 0; i < GetK_bucket_size(); i++)
    {
        if (tempKbuk[i] == "NULL")
        {
            tempKbuk[i] = sourceNode;
            outputPair.first = sourceNode;
            return outputPair;
        }
    }
    
    std::string output_KID = sourceNode;
    int replaceIndex = 0;
    
    for (int i = 0; i < GetK_bucket_size(); i++)
    {
        long int distance = XOR(output_KID, this->GetKId());
        if (XOR(tempKbuk[i], this->GetKId()) < distance)
        {
            output_KID = tempKbuk[i];
            replaceIndex = i;
        }
        
    }

    if (output_KID != sourceNode)
    {
        tempKbuk[replaceIndex] = sourceNode;
        outputPair.second = output_KID;
    }
    
    return outputPair;
    

}

//將輸入節點從k桶中去除
void
Kademlia::KBucket_delete(std::string sourceNode)
{
    for (int i = 0; i < GetK_bucket_size(); i++)
    {
        if (k_bucket[i] == sourceNode)
        {
            k_bucket[i] = "NULL";
            return;
        }
        if (k_bucket4[i] == sourceNode)
        {
            k_bucket4[i] = "NULL";
            return;
        }
        if (k_bucket8[i] == sourceNode)
        {
            k_bucket8[i] = "NULL";
            return;
        }
        
    }
    return;
}

//從DB中刪除資料
void
Kademlia::Delete_data(std::string DataName)
{
    Data* targetPtr = dataList->GetData(DataName, GetKId());

    if (targetPtr == NULL)
    {
        std::cout << "In delete_data(), GetData() is NULL\n";
        return;
    }

    // sqlite3 *db;
    char *zErrMsg = 0;
    int rc;
    std::string sqlCommand; 
    
    sqlCommand = std::string("DELETE from RECORD WHERE NODE=") + "'" + GetKId() + "'" + " AND DATA='" + DataName + "';";  
                 
    
    /* Execute SQL statement */
   rc = sqlite3_exec(db, &sqlCommand[0], dataList->DB_DeleteData, 0, &zErrMsg);
   if( rc != SQLITE_OK ){
   fprintf(stderr, "SQL error: %s\n", zErrMsg);
      sqlite3_free(zErrMsg);
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
    
    Data* prePtr = queryList;

    while (1)
    {
        if (prePtr->next == targetPtr)
        {
            // std::cout << "Delete Data sucess: " << targetPtr->Name << "\n";
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

void
Kademlia::SetK_bucket_to_DB(){
    //刪除DB中K桶資訊

    // sqlite3 *db;
    char *zErrMsg = 0;
    int rc;
    std::string sqlCommand; 
    
    sqlCommand = std::string("DELETE from KBUCKET WHERE NODE=") + "'" + GetKId() + "'" + ";";  
                 
    /* Execute SQL statement */
   rc = sqlite3_exec(db, &sqlCommand[0], dataList->DB_NULL, 0, &zErrMsg);
   if( rc != SQLITE_OK ){
    fprintf(stderr, "SQL error: %s\n", zErrMsg);
   }
    
    for (int i = 0; i < GetK_bucket_size(); i++)
    {
        if (k_bucket[i] == "NULL")
        {
            continue;
        }

        //將現有K桶資訊存入資料庫
        sqlCommand = std::string("INSERT INTO KBUCKET (NODE,KID)") +
                    "VALUES('"+ this->KId + "', '" + k_bucket[i] + "');";
                    
        /* Execute SQL statement */
        rc = sqlite3_exec(db, &sqlCommand[0], dataList->DB_NULL, 0, &zErrMsg);
        if( rc != SQLITE_OK ){
            fprintf(stderr, "SQL error: %s\n", zErrMsg);
        }
    }

    for (int i = 0; i < GetK_bucket_size(); i++)
    {
        if (k_bucket4[i] == "NULL")
        {
            continue;
        }

        //將現有K桶資訊存入資料庫
        sqlCommand = std::string("INSERT INTO KBUCKET (NODE,KID)") +
                    "VALUES('"+ this->KId + "', '" + k_bucket4[i] + "');";
                    
        /* Execute SQL statement */
        rc = sqlite3_exec(db, &sqlCommand[0], dataList->DB_NULL, 0, &zErrMsg);
        if( rc != SQLITE_OK ){
            fprintf(stderr, "SQL error: %s\n", zErrMsg);
        }
    }

    for (int i = 0; i < GetK_bucket_size(); i++)
    {
        if (k_bucket8[i] == "NULL")
        {
            continue;
        }

        //將現有K桶資訊存入資料庫
        sqlCommand = std::string("INSERT INTO KBUCKET (NODE,KID)") +
                    "VALUES('"+ this->KId + "', '" + k_bucket8[i] + "');";
                    
        /* Execute SQL statement */
        rc = sqlite3_exec(db, &sqlCommand[0], dataList->DB_NULL, 0, &zErrMsg);
        if( rc != SQLITE_OK ){
            fprintf(stderr, "SQL error: %s\n", zErrMsg);
        }
    }
    
}

//將資料庫中的K桶資訊轉移至變數中
void
Kademlia::Init_Kbucket(){
    char *zErrMsg = 0;
    int rc;
    std::string sqlCommand, sqlOutput; 
    char* command_output = (char *)calloc(5000,sizeof(char)) ;
    std::string s = "|";
    strcpy(command_output, &s[0]);

    sqlCommand = std::string("SELECT * from KBUCKET WHERE NODE=") + "'" + KId + "'" + " ;";  
    
    /* Execute SQL statement */
   rc = sqlite3_exec(db, &sqlCommand[0], this->DB_getDATA_string, &command_output, &zErrMsg);
   if( rc != SQLITE_OK ){
    fprintf(stderr, "SQL error: %s\n", zErrMsg);
    sqlite3_free(zErrMsg);
    return;
   }

    // std::cout << "string len = "<< command_output << "\n";
    std::string inputString(command_output);
    free(command_output);

    if (inputString == "|")
    {
        return ;
    }

    int head = 0, tail = 0;
    head = inputString.find_first_of("|", head);
    tail = inputString.find_first_of("|", head+1);
    std::string temp ;

    while (tail != -1)
    {
        temp = inputString.substr(head+1, tail-head-1);
        std::pair<std::string, std::string> update_result = KBucket_update(temp, GetSameBits(temp));
        if (update_result.second != "NULL")
        {
            std::cout << "Init Kbucket error in Node " << this->KId << " / " << update_result.second << " replaced\n";
        }

        // if (inputString.find_first_of("|", tail) == inputString.find_last_of("|", tail))
        // {
        //     return ;
        // }
        head = tail;
        tail = inputString.find_first_of("|", head+1);
    }

}

//針對輸入節點，比較所有更接近的資料，回傳整理的字串
//需要修改
std::string
Kademlia::Transform_Data(std::string thisNode, std::string targetNode)
{
    char *zErrMsg = 0;
    int rc;
    std::string sqlCommand, sqlOutput; 
    char* command_output = (char *)calloc(500000,sizeof(char)) ;
    std::string s = "|";
    strcpy(command_output, &s[0]);

    sqlCommand = std::string("SELECT * from RECORD WHERE NODE=") + "'" + KId + "'" + " ;";  
    
    /* Execute SQL statement */
   rc = sqlite3_exec(db, &sqlCommand[0], this->DB_getDATA_string, &command_output, &zErrMsg);
   if( rc != SQLITE_OK ){
   fprintf(stderr, "SQL error: %s\n", zErrMsg);
      sqlite3_free(zErrMsg);
   }

    // std::cout << "string len = "<< command_output << "\n";
    std::string inputString(command_output);
    free(command_output);

    if (inputString == "|")
    {
        return inputString;
    }

    int head = 0, tail = 0;
    head = inputString.find_first_of("|", head);
    tail = inputString.find_first_of("|", head+1);
    std::string temp;
    std::string output = "|";

    while (tail != -1)
    {
        temp = inputString.substr(head+1, tail-head-1);
        
        //確認此資料沒有存在於目標節點中，有則不進行transform
        int flag = 0;
        sqlCommand = std::string("SELECT * from RECORD WHERE NODE=") + "'" + targetNode + "'" + " AND DATA='" + temp + "';";
        rc = sqlite3_exec(db, &sqlCommand[0], this->DB_getDATA, &flag, &zErrMsg);
        if( rc != SQLITE_OK ){
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
            sqlite3_free(zErrMsg);
        }

        if (flag == 0)
        {
            std::size_t hashData = std::hash<std::string>{}(temp);
            std::string binaryData = std::bitset<16>(hashData).to_string();
            long int thisNode_distance = XOR(thisNode, binaryData);
            long int targetNode_distance = XOR(targetNode, binaryData);

            if (targetNode_distance > thisNode_distance)
            {
                output = output + temp + "|" ;
            }
        }
        
       
        head = tail;
        tail = inputString.find_first_of("|", head+1);
    }
    
    return output;
}



//---------------------Class Data-------------------------
//需要修改
void
Data::AddData(std::string inputName, std::string k_ID, std::string sourceNode, bool init)
{
    // sqlite3 *db;
    std::cout << "Insert Data " << inputName << " in " << k_ID << " sourceNode: " << sourceNode <<"\n"; 

    char *zErrMsg = 0;
    int rc;
    std::string sqlCommand;
    int command_output = 0;

    if (init == false && k_ID != sourceNode)
    {
        //若自身已經擁有資料，則將資料加回原本節點中
        sqlCommand = std::string("SELECT * from RECORD WHERE NODE=") + "'" + k_ID + "'" + " AND DATA='" + inputName + "';";  
        rc = sqlite3_exec(db, &sqlCommand[0], this->DB_getDATA, &command_output, &zErrMsg);
        if( rc != SQLITE_OK ){
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
            sqlite3_free(zErrMsg);
        }
        if (command_output == 1)
        {
            return;
        }
        
        //自身沒有資料，若原本節點也沒有資料，則轉移至其他節點中
        sqlCommand = std::string("SELECT * from RECORD WHERE NODE=") + "'" + sourceNode + "'" + " AND DATA='" + inputName + "';";  
        command_output = 0;
        /* Execute SQL statement */
        rc = sqlite3_exec(db, &sqlCommand[0], this->DB_getDATA, &command_output, &zErrMsg);
        if( rc != SQLITE_OK ){
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
            sqlite3_free(zErrMsg);
        }
        if (command_output == 0)
        {
            return;
        }

        sqlCommand = std::string("DELETE FROM RECORD WHERE ") +
                 "NODE='"+ sourceNode + "' AND DATA= '" + inputName + "';";
        
        // std::cout << sqlCommand <<"\n";
    
            /* Execute SQL statement */
        rc = sqlite3_exec(db, &sqlCommand[0], this->DB_DeleteData, &command_output, &zErrMsg);
        if( rc != SQLITE_OK ){
        fprintf(stderr, "SQL error: %s\n", zErrMsg);
            sqlite3_free(zErrMsg);
        }

        
    }
    
    
    sqlCommand = std::string("INSERT INTO RECORD (NODE,DATA)") +
                 "VALUES('"+ k_ID + "', '" + inputName + "');";
    
    // std::cout << sqlCommand <<"\n";
    
    /* Execute SQL statement */
   rc = sqlite3_exec(db, &sqlCommand[0], this->DB_addDATA, 0, &zErrMsg);
   if( rc != SQLITE_OK ){
   fprintf(stderr, "SQL error: %s\n", zErrMsg);
      sqlite3_free(zErrMsg);
   }

}

void
Data::AddData(std::string inputName, std::string inputType, int Empty)
{
    Data* tailptr = this;
    while (tailptr->next != NULL)
    {
        tailptr = tailptr->next;
    }

    Data *inputData = new Data();
    inputData->next = NULL;
    tailptr->next = inputData;
    inputData->head = this;
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
        if (DataName == tempptr->Name)
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

//從資料庫裡存取資料
Data*
Data::GetData(std::string DataName, std::string k_ID)
{
    //需要修改

    // sqlite3 *db;
    char *zErrMsg = 0;
    int rc;
    std::string sqlCommand, sqlOutput; 
    int command_output = 0;

    // rc = sqlite3_open("test.db", &db);

    // if( rc ){
    //     fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
    //     exit(0);
    // }
    
    sqlCommand = std::string("SELECT * from RECORD WHERE NODE=") + "'" + k_ID + "'" + " AND DATA='" + DataName + "';";  
    
    /* Execute SQL statement */
   rc = sqlite3_exec(db, &sqlCommand[0], this->DB_getDATA, &command_output, &zErrMsg);
   if( rc != SQLITE_OK ){
   fprintf(stderr, "SQL error: %s\n", zErrMsg);
      sqlite3_free(zErrMsg);
   }
//    sqlite3_close(db);
    // std::cout << "command output = " <<command_output << "\n";
   if (command_output)
   {
       return this;
   }
   else
   {
       return NULL;
   }
   
   
}

void
Data::update_nextHop(std::string inputNode)
{
    long int distance;
    std::size_t hashData = std::hash<std::string>{}(this->Name);
    std::string binaryData = std::bitset<16>(hashData).to_string();
    distance = XOR(binaryData,inputNode);
    std::pair<std::string, int>mostFar_node;
    mostFar_node.first = nextHop_list[0];
    mostFar_node.second = 0;

    if (distance <= XOR(binaryData, closest_node))
    {
        return;
    }

    for (int  i = 0; i < 3; i++)
    {
        if (this->nextHop_list[i] == inputNode)
        {
            // 預防nextHop list出現相同節點，重複Query
            return;
        }

        if (this->nextHop_list[i] == "NULL")
        {
            this->nextHop_list[i] = inputNode;
            return;
        }

        if (XOR(mostFar_node.first, binaryData) > XOR(nextHop_list[i], binaryData))
        {
            mostFar_node.first = nextHop_list[i];
            mostFar_node.second = i;
        }
    }
    
    if (distance > XOR(mostFar_node.first, binaryData))
    {
        nextHop_list[mostFar_node.second] = inputNode;
    }
    return;
}

long int
Data::XOR(std::string input, std::string input2)
{
    // std::cout << "Xor data: " << input2 << " and " << input << "\n";
    long int distance = 0;
    for (int i = 0; i < 16; i++)
    {
        std::string str1 = std::to_string(input2[i]);
        std::string str2 = std::to_string(input[i]);
        if (str1 == str2)
        {
            distance = distance + pow(2, 16-i);
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

