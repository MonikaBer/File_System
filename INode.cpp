//Class representing i-node
#include <algorithm>

#include "INode.hpp"


//methods definitions
std::fstream& operator>>(std::fstream &inodes, std::shared_ptr<INode> iNode) {
    inodes.read((char*)&(iNode->mode), sizeof(iNode->mode));
    inodes.read((char*)&(iNode->length), sizeof(iNode->length));
    inodes.read((char*)&(iNode->number_of_blocks), sizeof(iNode->number_of_blocks));
    inodes.read((char*)iNode->blocks.data(), sizeof(iNode->blocks));
    inodes.read((char*)&(iNode->indirect_block), sizeof(iNode->indirect_block));
    return inodes;
}

std::fstream & operator<<(std::fstream &inodes, const std::shared_ptr<INode> iNode) {
    inodes.write((char*)&(iNode->mode), sizeof(iNode->mode));
    inodes.write((char*)&(iNode->length), sizeof(iNode->length));
    inodes.write((char*)&(iNode->number_of_blocks), sizeof(iNode->number_of_blocks));
    inodes.write((char*)iNode->blocks.data(), sizeof(iNode->blocks));
    inodes.write((char*)&(iNode->indirect_block), sizeof(iNode->indirect_block));
    return inodes;
}

INode::INode(unsigned int id, unsigned short mode, long length, unsigned int numberOfBlocks, unsigned int indirectBlock)
                : inode_id(id), mode(mode), length(length), number_of_blocks(numberOfBlocks), indirect_block(indirectBlock) {}

INode::INode(unsigned id): inode_id(id) {
    ConfigLoader * config = ConfigLoader::getInstance();
    std::fstream & inodes = config->getInodes();
    unsigned int inode_position = id * sizeofInode;
    inodes.seekg(0, std::ios::end);
    if(inodes.tellg() < inode_position + sizeofInode)
        throw std::runtime_error("inode doesnt exist");
    inodes.seekg(inode_position);
    inodes >> std::shared_ptr<INode>(this);
}

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
            // little endian
            inode_id += 0xFF000000 & *(inode_content.begin()+i+maxFileName+4) << 24;
            inode_id += 0x00FF0000 & *(inode_content.begin()+i+maxFileName+3) << 16;
            inode_id += 0x0000FF00 & *(inode_content.begin()+i+maxFileName+2) << 8;
            inode_id += 0x000000FF & *(inode_content.begin()+i+maxFileName+1);
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

unsigned int INode::getId() const {
    return inode_id;
}

void INode::save(std::string newFileName, INode newFileInode) {
    addFileToDirectory(newFileName, newFileInode);
    ConfigLoader * config = ConfigLoader::getInstance();
    std::fstream & inodes = config->getInodes();
    inodes.seekg(newFileInode.getId()*sizeofInode);
    inodes << std::make_shared<INode>(newFileInode);
    std::fstream &inodesBitmap = config->getInodesBitmap();
}

std::array<char, INode::sizeofInode> INode::serialize() {
    std::array<char, sizeofInode> INodeBytes = {0};
    int i = 0;

    for(; i<sizeof(mode); ++i)
        INodeBytes[i] = (mode >> (i * 8));

    for(int k=0; i<sizeof(length); ++i, ++k)
        INodeBytes[i] = (length >> (k * 8));

    for(int k=0; i<sizeof(number_of_blocks); ++i, ++k)
        INodeBytes[i] = (number_of_blocks >> (k * 8));

    for(auto & block : blocks)
        for(int k =0; i<sizeof(block); ++i, ++k)
            INodeBytes[i] = (block >> (k * 8));

    for(int k =0; i<sizeof(indirect_block); ++i, ++k)
        INodeBytes[i] = (indirect_block >> (k * 8));

    return INodeBytes;
}

void INode::addFileToDirectory(std::string newFileName, INode inode) {
    ConfigLoader* loader = ConfigLoader::getInstance();
    if(newFileName.size() > loader->getMaxLengthOfName())
        throw std::runtime_error("Bad length of file name");
    std::fstream& blocks = loader->getBlocks();
    unsigned positionInBlock = length%loader->getSizeOfBlock();
    unsigned blockId = length/loader->getSizeOfBlock();
    char *savedFileName = new char[loader->getMaxLengthOfName()];
    newFileName.copy(savedFileName, newFileName.size());
    if(newFileName.size() != loader->getMaxLengthOfName())
        savedFileName[newFileName.size()] = 0;
    unsigned inodeId = inode.getId();
    blocks.seekg(getBlock(blockId)*loader->getSizeOfBlock()+positionInBlock);
    blocks.write(savedFileName, loader->getMaxLengthOfName());
    blocks.write((char*)&(inodeId), sizeof(inodeId));
    delete savedFileName;
}
