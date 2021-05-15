
#include <iostream>
#include <string>
#include "kademlia.hpp"

Kademlia::Kademlia(std::string node_name_input, std::string data_input)
{
    node_name = node_name_input;
    data = data_input;
}

bool
Kademlia::GetData(std::string input){
    return data.compare(input);
}

Kademlia *
Kademlia::GetNext_Node(){
    return next_node;
}

void
Kademlia::SetNext(Kademlia *input){
    next_node = input;
}