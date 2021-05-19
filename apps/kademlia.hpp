
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

    Kademlia(std::string node_name_input, std::string data_input, int kademliaID);

    bool
    GetData(std::string input);

    void
    SetData(std::string input);

    Kademlia*
    GetNext_Node(std::string TargetNode);

    std::string
    GetKId(){
        return std::to_string(KId);
    };

    void
    Set_KBucket(Kademlia *KNode);

    void
    Node_info();

private:
    std::string node_name;
    int KId;
    Data *dataList;
    Kademlia *k_bucket[5] = {NULL, NULL, NULL, NULL, NULL};
    int knum = 0;
};



#endif 