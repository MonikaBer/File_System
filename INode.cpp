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

INode::INode(unsigned int id, unsigned short mode, long length, unsigned int numberOfBlocks, unsigned int indirectBlock)
                : inode_id(id), mode(mode), length(length), number_of_blocks(numberOfBlocks), indirect_block(indirectBlock) {}

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

std::map<std::string, unsigned> INode::getDirectoryContent() {
    if(!mode)
        throw std::runtime_error("INode is not a directory");

    std::map<std::string, unsigned> dir_content;
    std::vector<char> inode_content = getContent();
    ConfigLoader * config = ConfigLoader::getInstance();
    int maxFileName = config->getMaxLengthOfName();
    for(unsigned long i=0; i<length; i += maxFileName+sizeof(unsigned)){
            std::string name(inode_content.begin()+i, inode_content.begin()+i+maxFileName);
            unsigned inode_id = 0;
            inode_id += 0xFF000000 & *(inode_content.begin()+i+maxFileName+1);
            inode_id += 0x00FF0000 & *(inode_content.begin()+i+maxFileName+2);
            inode_id += 0x0000FF00 & *(inode_content.begin()+i+maxFileName+3);
            inode_id += 0x000000FF & *(inode_content.begin()+i+maxFileName+4);
            dir_content.insert(std::make_pair(name, inode_id));
    }
    return dir_content;
}

std::vector<char> INode::getContent() {
    ConfigLoader * config = ConfigLoader::getInstance();
    std::fstream & blocks_stream = config->getBlocks();
    int sizeOfBlock = config->getSizeOfBlock();
    std::vector<char> content;
    unsigned long loaded = 0;
    char block_content[sizeOfBlock];
    //todo: pewnie trzeba tu zrobić blokady
    for(auto & a : blocks){
        blocks_stream.seekg(a * sizeOfBlock);
        if(length - loaded < sizeOfBlock){
            if(length - loaded) {
                blocks_stream.read(block_content, length - loaded);
                content.insert(content.end(), block_content, block_content+length-loaded);
            }
            break;
        }
        blocks_stream.read(block_content, sizeOfBlock);
        content.insert(content.end(), block_content, block_content+sizeOfBlock);
        loaded += sizeOfBlock;
    }
}
