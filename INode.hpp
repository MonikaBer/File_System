//Class representing i-node
#ifndef I_NODE_HPP
#define I_NODE_HPP
#include <vector>

class INode {
private:
    unsigned short mode;
    long length;
    unsigned number_of_blocks;
    std::vector<unsigned> blocks;
    unsigned indirect_block;

public:
    //methods declarations
};

#endif