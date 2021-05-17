
#ifndef KADEMLIA_
#define KADEMLIA_

#include <iostream>

class Kademlia{

public:
    Kademlia();

    Kademlia(std::string node_name_input, std::string data_input);

    bool
    GetData(std::string input);

    Kademlia*
    GetNext_Node();

    void
    SetNext(Kademlia *input);

    void
    Set_KBucket(std::string input);

    void
    Node_info();

private:
    std::string node_name;
    Kademlia *next_node;
    std::string data;
    std::string k_bucket;
};


#endif 