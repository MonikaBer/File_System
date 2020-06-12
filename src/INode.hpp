//Class representing i-node
#ifndef I_NODE_HPP
#define I_NODE_HPP

#include <iostream>
#include <array>
#include "ResourceManager.hpp"
#include <fstream>
#include <vector>

class INode {
private:
    const static unsigned maxBlocksNo = 12;

    unsigned short mode = 0;        //0 - file, 1 - dir
    long length = 0;
    unsigned number_of_blocks = 0;
    std::array<unsigned, maxBlocksNo> blocks = {0};
    unsigned indirect_block = 0;

    unsigned int inode_id;

    void addFileToDirectory(std::string newFileName, INode inode);

    void loadInode(INode* en, int nodeId);
    void saveInode(const INode* en);

    int addBlock(unsigned block);
    int removeBlock();

    unsigned getFreeBlock();
    int freeBlock(unsigned block);

public:
    static const int sizeofInode = sizeof(unsigned short) + sizeof(long) + 14 * sizeof(unsigned);

    // TODO public for testing

    unsigned int getId() const;

    //methods declarations
    INode() = default;
    INode(unsigned id); // reads inode from file
    INode(unsigned int id, unsigned short mode, long length, unsigned int numberOfBlocks, unsigned int indirectBlock); //TODO: Change constructor

    long getLength() const { return length; };

    int writeToFile(char* buffer, int size, long fileCursor);
    void saveINodeInDirectory(std::string newFileName, INode newFileInode);


    std::map<std::string, unsigned> getDirectoryContent();
    std::vector<char> getContent();
    int writeInode();
    unsigned short getMode();

    unsigned getBlock(unsigned index) const;

    int freeAllBlocks();

    void removeFileFromDirectory(INode inode);
};

#endif