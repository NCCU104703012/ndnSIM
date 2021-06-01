
#ifndef KADEMLIA_
#define KADEMLIA_

#include <iostream>

class Data{

public:
    ~Data()
    {
        head = this;
    }

    void AddData(std::string inputName);

    Data* GetData(std::string DataName);

    void printAllData();

    Data* GetTail();

    Data *head;
    Data *next = NULL;
    std::string Name = "NULL";
    std::string UserID;
    std::string age;
    std::string item;
};

class Kademlia{

public:
    Kademlia();

    Kademlia(std::string node_name_input, std::string data_input, std::string kademliaID);

    bool
    GetData(std::string DataName);

    void
    SetData(std::string input);

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
    Kademlia *k_bucket[5] = {NULL, NULL, NULL, NULL, NULL};
    int knum = 0;
};



#endif 