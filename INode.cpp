//Class representing i-node
#include <algorithm>

#include "INode.hpp"


//methods definitions
std::istream & operator>>(std::istream &is, INode &iNode) {
    is.read((char*)&iNode.mode, sizeof(iNode.mode));
    is.read((char*)&iNode.length, sizeof(iNode.length));
    is.read((char*)&iNode.number_of_blocks, sizeof(iNode.number_of_blocks));
    is.read((char*)iNode.blocks.data(), sizeof(iNode.blocks));
    is.read((char*)&iNode.indirect_block, sizeof(iNode.indirect_block));
    return is;
}

std::ostream & operator<<(std::ostream &os, const INode &iNode) {
    os.write((char*)&iNode.mode, sizeof(iNode.mode));
    os.write((char*)&iNode.length, sizeof(iNode.length));
    os.write((char*)&iNode.number_of_blocks, sizeof(iNode.number_of_blocks));
    os.write((char*)iNode.blocks.data(), sizeof(iNode.blocks));
    os.write((char*)&iNode.indirect_block, sizeof(iNode.indirect_block));
    return os;
}

INode::INode(unsigned short mode, long length, unsigned int numberOfBlocks, unsigned int indirectBlock)
                : mode(mode), length(length), number_of_blocks(numberOfBlocks), indirect_block(indirectBlock) {}

int INode::addBlock(unsigned int block) {
    auto it = std::find(blocks.begin(), blocks.end(), 0);
    if (it == blocks.end()) {
        // TODO implement adding more than 12 blocks
        if (indirect_block == 0) {
            // TODO get block for your use
        } else {
            // TODO HOW DO I READ FROM FILES??? std::fstream blocks(blocks, std::ios::binary | std::ios::in | std::ios::out);
        }
        return -123;
    }
    else {
        *it = block;
    }
    return 0;
}

int INode::removeBlock(unsigned int block) {
    auto it = std::remove(blocks.begin(), blocks.end(), block);
    if (it == blocks.end())
        // TODO implement adding more than 12 blocks
        return -123;
    else {
        *it = 0;    // will not work if std::removes removes more than one value, but that means inode was corrupted
    }
    return 0;
}

int INode::freeAllBlocks() {
    // TODO Implement
    return 0;
}
