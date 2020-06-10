//Class representing i-node
#include <algorithm>
#include <cstring>

#include "INode.hpp"
#include <unistd.h>

void INode::loadInode(INode* en, int inodeId){
    int inodesFileDescriptor = ConfigLoader::getInstance()->getInodes();
    unsigned int inode_position = inodeId * sizeofInode;
    lseek(inodesFileDescriptor, 0, SEEK_END);
    if(lseek(inodesFileDescriptor, 0, SEEK_CUR) < inode_position + sizeofInode)
        throw std::runtime_error("inode doesnt exist");
    lseek(inodesFileDescriptor, inode_position, SEEK_SET);

    read(inodesFileDescriptor, (char*)&(en->mode), sizeof(en->mode));
    read(inodesFileDescriptor, (char*)&(en->length), sizeof(en->length));
    read(inodesFileDescriptor, (char*)&(en->number_of_blocks), sizeof(en->number_of_blocks));
    read(inodesFileDescriptor, (char*)en->blocks.data(), sizeof(en->blocks));
    read(inodesFileDescriptor, (char*)&(en->indirect_block), sizeof(en->indirect_block));
}

void INode::saveInode(const INode* en){
    int inodesFileDescriptor = ConfigLoader::getInstance()->getInodes();
    lseek(inodesFileDescriptor, en->getId()*sizeofInode, SEEK_SET);
    write(inodesFileDescriptor, (char*)&(en->mode), sizeof(en->mode));
    write(inodesFileDescriptor, (char*)&(en->length), sizeof(en->length));
    write(inodesFileDescriptor, (char*)&(en->number_of_blocks), sizeof(en->number_of_blocks));
    write(inodesFileDescriptor, (char*)en->blocks.data(), sizeof(en->blocks));
    write(inodesFileDescriptor, (char*)&(en->indirect_block), sizeof(en->indirect_block));
}

INode::INode(unsigned int id, unsigned short mode, long length, unsigned int numberOfBlocks, unsigned int indirectBlock)
                : inode_id(id), mode(mode), length(length), number_of_blocks(numberOfBlocks), indirect_block(indirectBlock) {}

INode::INode(unsigned id): inode_id(id) {
    ConfigLoader * config = ConfigLoader::getInstance();
    loadInode(this, id);
}

/**
 * Adds block of data to iNode. Block can be acquired by ConfigLoader::getInstance()->getFreeBlock().
 *
 * @param block - block index
 * @return
 */
int INode::addBlock(unsigned int block) {
    if(number_of_blocks < 12) {
        blocks[number_of_blocks] = block;
    }
    else {
        if (indirect_block == 0) {
            indirect_block = ConfigLoader::getInstance()->getFreeBlock();  // TODO Artur olockuj to jakos
        }
        int blocksFile = ConfigLoader::getInstance()->getBlocks();
        unsigned long host_file_offset = indirect_block * 4096 + (number_of_blocks - 12) * sizeof(unsigned);
        lseek(blocksFile, host_file_offset, SEEK_SET);
        write(blocksFile, (char*)&block, sizeof(unsigned));
    }
    number_of_blocks++;
    return 0;
}

int INode::removeBlock() {
    if(number_of_blocks <= 12) {
        ConfigLoader::getInstance()->freeBlock(blocks[number_of_blocks - 1]);
        blocks[number_of_blocks - 1] = 0;
    }
    else {
        int blocksFile = ConfigLoader::getInstance()->getBlocks();
        unsigned long host_file_offset = indirect_block * 4096 + (number_of_blocks - 13) * sizeof(unsigned);
        lseek(blocksFile, host_file_offset, SEEK_SET);
        unsigned block;
        read(blocksFile, (char*)&block, sizeof(unsigned));
        ConfigLoader::getInstance()->freeBlock(block);
        if (number_of_blocks == 13) {
            ConfigLoader::getInstance()->freeBlock(indirect_block);
            indirect_block = 0;     // TODO Artur zwolnij z tego locka czy cos nie znam sie XD
        }
    }
    number_of_blocks--;
    return 0;
}

int INode::freeAllBlocks() {
    while (number_of_blocks > 0)
        removeBlock();
    return 0;
}

std::map<std::string, unsigned> INode::getDirectoryContent() {
//    if(!mode)
//        throw std::runtime_error("INode is not a directory");

    std::map<std::string, unsigned> dir_content;
    std::vector<char> inode_content = getContent();
    ConfigLoader * config = ConfigLoader::getInstance();
    int maxFileName = config->getMaxLengthOfName();
    for(unsigned long i=0; i<length; i += maxFileName+sizeof(unsigned)){
            std::string name = (inode_content.data() + i);
            unsigned inode_id = 0;
            std::memcpy(&inode_id, inode_content.data()+i+maxFileName, sizeof(unsigned));
            dir_content.insert(std::make_pair(name, inode_id));
    }
    return dir_content;
}

std::vector<char> INode::getContent() {
    ConfigLoader * config = ConfigLoader::getInstance();
    int blocks_stream = config->getBlocks();
    int sizeOfBlock = config->getSizeOfBlock();
    std::vector<char> content;
    unsigned long loaded = 0;
    char block_content[sizeOfBlock];
    //todo: pewnie trzeba tu zrobić blokady
    for(auto & a : blocks){
        lseek(blocks_stream, a*sizeOfBlock, SEEK_SET);
        if(length - loaded < sizeOfBlock){
            if(length - loaded) {
                read(blocks_stream, block_content, length - loaded);
                content.insert(content.end(), block_content, block_content+length-loaded);
            }
            break;
        }
        read(blocks_stream, block_content, sizeOfBlock);
        content.insert(content.end(), block_content, block_content+sizeOfBlock);
        loaded += sizeOfBlock;
    }
    return content;
}

unsigned int INode::getId() const {
    return inode_id;
}

void INode::saveINodeInDirectory(std::string newFileName, INode newFileInode) {
    addFileToDirectory(newFileName, newFileInode);
    ConfigLoader * config = ConfigLoader::getInstance();
    saveInode(&newFileInode);
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
    int blocks = loader->getBlocks();
    unsigned positionInBlock = length%loader->getSizeOfBlock();
    unsigned blockId = length/loader->getSizeOfBlock();
    char *savedFileName = new char[loader->getMaxLengthOfName()];
    newFileName.copy(savedFileName, newFileName.size());
    if(newFileName.size() != loader->getMaxLengthOfName())
        savedFileName[newFileName.size()] = 0;
    for (int i = newFileName.size(); i < loader->getMaxLengthOfName(); i++) {
        savedFileName[i] = 0;
    }
    unsigned inodeId = inode.getId();
    lseek(blocks, getBlock(blockId)*loader->getSizeOfBlock()+positionInBlock, SEEK_SET);
    write(blocks, savedFileName, loader->getMaxLengthOfName());
    write(blocks, (char*)&(inodeId), sizeof(inodeId));
    delete savedFileName;
    length += loader->getMaxLengthOfName() + sizeof(inodeId);
    writeInode();
}


/**
 * Saves INode to file and sets bit in bitmap to inform that INode is in use.
 *
 * @param fd - file descriptor holding INode number of file to save
 * @param inode - object to save
 * @return
 */
int INode::writeInode() {
    ConfigLoader* loader = ConfigLoader::getInstance();
    saveInode(this);
    // update bitmap
    // TODO maybe split into updateInode (wont change bitmap) and createInode, which will update bitmap
    int inodesBitmap = loader->getInodesBitmap();
    lseek(inodesBitmap, getId()/8, SEEK_SET);
    char byte;
    read(inodesBitmap, &byte, 1);
    byte |= 1 << (getId()%8);
    lseek(inodesBitmap, getId()/8, SEEK_SET);
    write(inodesBitmap, &byte, 1);
    // TODO return errors ( + doc)
}

int INode::writeToFile(char *buffer, int size, long fileCursor) {
    ConfigLoader * config = ConfigLoader::getInstance();
    int blocks_stream = config->getBlocks();
    const int sizeOfBlock = config->getSizeOfBlock();
    long positionInBlock = fileCursor%sizeOfBlock;
    int saved = 0;

    for(int blockIndex = fileCursor/sizeOfBlock; size > 0; blockIndex++){
        int blockAddress = blocks[blockIndex];

        if (blockAddress == 0) {
            unsigned freeBlock = config->getFreeBlock();
            if (freeBlock == 0) throw std::runtime_error("COULDNT FIND FREE BLOCK");
            blocks[blockIndex] = freeBlock;
            blockAddress = blocks[blockIndex];
        }
        lseek(blocks_stream, blockAddress*sizeOfBlock+positionInBlock, SEEK_SET);
        int remainingSizeOfBlock = sizeOfBlock-positionInBlock;
        if (remainingSizeOfBlock<=size){ //TODO Check equals
            write(blocks_stream, buffer + saved, remainingSizeOfBlock);
            saved += remainingSizeOfBlock;
            size -= remainingSizeOfBlock;
        }
        else {
            write(blocks_stream, buffer + saved, size);
            saved += size;
            size = 0;
        }
        positionInBlock=0;
    }
    if (fileCursor + saved > length)
        length = fileCursor + saved;
    return saved;
}
