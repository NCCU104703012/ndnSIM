
#ifndef KADEMLIA_
#define KADEMLIA_

#include <iostream>
#include <set>

class Order
{
private:
    /* data */
    std::string orderName;
    double timeStamp;
    int targetNum;
    int tempNum;
    int fulfill_data_num;
    Order* nextPtr;
    std::set<std::string> dataList = {};

public:
    Order(std::string dataString, double timeString, int targetNumber)
    {
    orderName = dataString;
    timeStamp = timeString;
    targetNum = targetNumber;
    tempNum = 0;
    fulfill_data_num = 0;
    nextPtr = NULL;
    };

    std::string
    getOrderName(){
        return orderName;
    };

    double
    getTimeStamp(){
        return timeStamp;
    };

    int
    getTargetNum(){
        return targetNum;
    };

    std::set<std::string>
    getDataList(){
        return dataList;
    }

    void
    setDataList(std::string input){
        dataList.insert(input);
    }


    Order* getNext(){
        return nextPtr;
    };

    void
    setNext(Order* ptr);

    void
    AddOrder(std::string dataString, double timeString, int targetNumber);

    Order* getTail();
    ~Order(){};
};


class Data{

public:
    ~Data()
    {
        head = this;
    }

    void AddData(std::string inputName, std::string inputType);

    Data* GetData(std::string DataName);

    void printAllData();

    Data* GetTail();

    Data *head;
    Data *next = NULL;
    std::string Name = "NULL";
    std::string UserID;
    std::string age;
    std::string item;
    std::string type;
};

class Kademlia{

public:
    Kademlia();

    Kademlia(std::string node_name_input, std::string data_input, std::string kademliaID);

    bool
    GetData(std::string DataName);

    void
    SetData(std::string input, std::string type);

    Kademlia*
    GetNext_Node(std::string TargetNode);

    int
    XOR(std::string input);

    std::string
    GetKId(){
        return KId;
    };

    void
    Set_KBucket(Kademlia *KNode);

    void
    Node_info();

    //query資料時呼叫 決定下一個節點
    Kademlia* QueryKbucket(std::string DataName)
    {
        return k_bucket[0];
    };

private:
    std::string node_name;
    std::string KId;
    Data *dataList;
    Kademlia *k_bucket[15] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
    int knum = 0;
};



#endif 