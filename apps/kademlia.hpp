
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

private:
    std::string node_name;
    Kademlia *next_node;
    std::string data;

};


#endif 