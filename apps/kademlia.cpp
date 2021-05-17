
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

void
Kademlia::Node_info(){
    std::cout << "**************" << std::endl;
    std::cout << "node name = " << node_name << std::endl;
    std::cout << "data = " << data << std::endl;
    std::cout << "next node = " << next_node << std::endl;
    std::cout << "**************" << std::endl;
}

void
Kademlia::Set_KBucket(std::string input)
{
    k_bucket = input;
}