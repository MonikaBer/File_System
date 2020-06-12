//Main class of file system
#include <fstream>
#include <cmath>
#include "SimpleFS.hpp"
#include "ResourceManager.hpp"
#include <functional>
#include "Lock.hpp"
#include <unistd.h>
#include <stdio.h>
#include <string.h>

SimpleFS::SimpleFS(std::string path){
    ResourceManager::initialize(path);
}

int SimpleFS::create(std::string && path, unsigned short mode) {
    //std::stack<Lock> inodeLocks; //TODO: Locally?
    std::vector<std::string> parsedPath = parseDirect(path);
    if(parsedPath.empty())
        return -1;
    std::string newFileName = parsedPath.back();
    parsedPath.pop_back();
    INode targetDirINode;
    try{
        targetDirINode = getTargetDirectory(parsedPath);
    }catch(...){
        while(!openInodes.empty())
            openInodes.pop();
        return -1;
    }
    int freeInodeId = findFreeInode();
    if(freeInodeId < 0) {
        while(!openInodes.empty())
            openInodes.pop();
        return -1;
    }

    std::map<std::string, unsigned> dirContent = targetDirINode.getDirectoryContent();
    if(dirContent.find(newFileName) != dirContent.end()) {
        while(!openInodes.empty())
            openInodes.pop();
        return -1;
    }
    INode newFile = INode(freeInodeId, mode, 0, 0, 0); //todo numberofblocks and indirectblock ????
    targetDirINode.saveINodeInDirectory(newFileName, newFile);
    openInodes.emplace(Lock::WR_LOCK, freeInodeId);
    newFile.writeInode();

    while(!openInodes.empty())
        openInodes.pop();
    return 0;
}


int SimpleFS::_open(std::string && path, int mode) {
    std::vector<std::string> parsedPath = parseDirect(path);
    if(parsedPath.empty())
        return -1;
    std::string fileName = parsedPath.back();
    parsedPath.pop_back();
    try{
        INode targetDirINode = getTargetDirectory(parsedPath);
        std::map<std::string, unsigned> dirContent = targetDirINode.getDirectoryContent();
        std::shared_ptr<INode> openINode = std::make_shared<INode>(dirContent.at(fileName));

        if (mode == 1)
            fds.emplace_back(openINode, Lock::WR_LOCK);
        else
            fds.emplace_back(openINode, Lock::RD_LOCK);

    } catch(std::runtime_error& e){
        std::cout << e.what();
        return -1;
    } catch(std::out_of_range& e){
        return -1;
    }


    return fds.size()-1;
}


int SimpleFS::_read(int fd, char * buf, int len) {
    if ( fd >= fds.size() )
        return -1;
    if(len < 0)
        return -1;

    FileDescriptor &fdr = fds[fd];
    unsigned cursor = fdr.getFileCursor();
    std::shared_ptr<INode> inode = fdr.getInode();
    unsigned totalRead = 0;

    int blocksFile = ResourceManager::getInstance()->getBlocks();

    // if trying to read more than left in file, cut len so it stops on end of file
    if (len + cursor > inode->getLength())
        len = inode->getLength() - cursor;

    int blockSize = ResourceManager::getInstance()->getSizeOfBlock();
    while (len) {
        // calculate host file offset
        unsigned iNodeBlockIndex = cursor / blockSize;
        unsigned blockOffset = cursor % blockSize;
        unsigned block = inode->getBlock(iNodeBlockIndex);
        unsigned long host_file_offset = block * blockSize + blockOffset;

        unsigned singleRead = len;
        // if this read spans over more than 1 block, read must be divided into parts
        if (blockOffset + len > blockSize)
            singleRead = blockSize - blockOffset;

        lseek(blocksFile, host_file_offset, SEEK_SET);
        read(blocksFile, buf+totalRead, singleRead);

        totalRead += singleRead;
        cursor += singleRead;
        len -= singleRead;
    }

    return totalRead;
}


int SimpleFS::_write(int fd, char * buf, int len) {
    FileDescriptor descriptor = fds[fd];
    return descriptor.writeToInode(buf, len);
}


int SimpleFS::_lseek(int fd, int whence, int offset) {
    if ( fd >= fds.size() )
        throw std::runtime_error("File descriptor doesn't exist");

    FileDescriptor &fdr = fds[fd];
    std::shared_ptr<INode> inode = fdr.getInode();

    if (whence == 0) {
        if (offset < 0 || offset >= inode->getLength())
            throw std::runtime_error("Wrong offset value");
        fdr.setFileCursor(offset);
    }
    else if (whence == 1) {
        long cursor = fdr.getFileCursor();
        if (offset + cursor < 0 || offset + cursor >= inode->getLength()) {
            throw std::runtime_error("Cannot move by offset from current position");
        }
        fdr.setFileCursor(offset + cursor);
    }
    else if (whence == 2) {
        if (offset > 0 || offset + inode->getLength() < 0) {
            throw std::runtime_error("Cannot move by offset from end position");
        }
        // - 1 because length points to byte AFTER last byte of file, which is against lseek description in documentation
        fdr.setFileCursor(inode->getLength() - 1 + offset);
    }
    else throw std::runtime_error("Wrong whence value");
    return fdr.getFileCursor();
}


int SimpleFS::unlink(std::string && name) {
    std::vector<std::string> parsedPath = parseDirect(name);
    INode targetDirINode;
    if(parsedPath.empty())
        return -1;
    std::string fileToDelete = parsedPath.back();
    parsedPath.pop_back();
    try{
        targetDirINode = getTargetDirectory(parsedPath);
    }catch(...){
        return -1;
    }
    std::map<std::string, unsigned> dirContent = targetDirINode.getDirectoryContent();
    unsigned iNodeToDeleteId;
    try {
        iNodeToDeleteId = dirContent.at(fileToDelete); // TODO LOCKI
    }catch(...){
        return -1;
    }
    INode inodeToDelete(iNodeToDeleteId);
    inodeToDelete.freeAllBlocks();
    clearInode(inodeToDelete.getId()); // TODO locks
    return 0;
}


int SimpleFS::mkdir(std::string && path) {
    std::vector<std::string> parsedPath = parseDirect(path);
    if(parsedPath.empty())
        return -1;
    std::string newDirName = parsedPath.back();         //name of new directory
    parsedPath.pop_back();
    INode targetDirINode;
    try {
        targetDirINode = getTargetDirectory(parsedPath);    //parent directory for new directory
    } catch (...) {
        return -1;
    }
    int freeInodeId = findFreeInode();
    if(freeInodeId < 0)
        return -1;
    INode newDir = INode(freeInodeId, 1, 0, 0, 0); //todo numberofblocks and indirectblock ????
    targetDirINode.saveINodeInDirectory(newDirName, newDir);
    return 0;
}


int SimpleFS::rmdir(std::string && path) {
    std::vector<std::string> parsedPath = parseDirect(path);
    INode targetDirINode;
    if(parsedPath.empty())
        return -1;

    std::string dirToDelete = parsedPath.back();
    parsedPath.pop_back();
    try {
        targetDirINode = getTargetDirectory(parsedPath);
    } catch(...) {
        return -1;
    }

    std::map<std::string, unsigned> dirContent = targetDirINode.getDirectoryContent();
    unsigned dirToDeleteId;
    try{
        dirToDeleteId = dirContent.at(dirToDelete); // TODO LOCKI
    } catch(...){
        return -1;
    }
    INode dirToDeleteINode(dirToDeleteId);

    if (dirToDeleteINode.getMode() == 0)  //it's file, not directory
        return -2;

    //check if dirToDelete is empty
    std::map<std::string, unsigned> dirToDeleteContent = dirToDeleteINode.getDirectoryContent();
    if (!dirToDeleteContent.empty())
        return -3;

    //dirToDelete is empty, so delete it
    dirToDeleteINode.freeAllBlocks();                //clear data blocks
    clearInode(dirToDeleteINode.getId());            //clear inode in bitmap     // TODO locks

    return 0;
}

INode SimpleFS::getTargetDirectory(const std::vector<std::string> &path) {
    openInodes.emplace(Lock::RD_LOCK, 0);
    INode inode = INode(0);
    for(const auto & fileName : path){
        std::map<std::string, unsigned> dir = inode.getDirectoryContent();
        int targetInodeNumber = dir.at(fileName);
        openInodes.emplace(Lock::RD_LOCK, targetInodeNumber);
        inode = INode(targetInodeNumber);
    }
    return inode;
}

std::vector<std::string> SimpleFS::parseDirect(const std::string& path) {
    if(path.empty() || path[0] != '/')
        return {};
    std::vector<std::string> parsed_path;
    std::string path_element;
    for (auto a = path.begin() + 1; a != path.end(); ++a) {
        if (*a == '/') {
            parsed_path.push_back(path_element);
            path_element.clear();
            continue;
        }
        path_element.push_back(*a);
        if (a == path.end() - 1 ) {
            parsed_path.push_back(path_element);
        }
    }
    return parsed_path;
}

int SimpleFS::findFreeInode() {
    int input = ResourceManager::getInstance()->getInodesBitmap();
    size_t sizeOfLine = 0;
    char * line = NULL;
    FILE *stream = fdopen(input, "wb+");
    if(getline(&line, &sizeOfLine, stream) == -1)
        return -1;
    std::size_t free_inode_byte = 0;
    while(free_inode_byte < sizeOfLine){
        if((unsigned char)line[free_inode_byte] != 0xFF)
            break;
        free_inode_byte++;
    }
    if((unsigned char)line[free_inode_byte] == 0xFF)
        return -1;
    char byteWithFreeINode = line[free_inode_byte];
    unsigned int id = 0;
    while((byteWithFreeINode>>id) & 0x1)
        ++id;
    byteWithFreeINode |= 1<<id;
    lseek(input, free_inode_byte, SEEK_SET);
    write(input, (char*)&byteWithFreeINode, 1);
    return 8*free_inode_byte+id;
}


/**
 * Clears INode by clearing corresponding bit in bitmap. Data is not cleared.
 *
 * @param fd - file descriptor holding INode number of file to delete
 * @return 
 */
int SimpleFS::clearInode(unsigned inode) {
    int bitmapfs = ResourceManager::getInstance()->getInodesBitmap();
    lseek(bitmapfs, inode/8, SEEK_SET);
    char byte;
    read(bitmapfs, &byte, 1);
    byte &= ~(1 << (inode%8));
    lseek(bitmapfs, inode/8, SEEK_SET);
    write(bitmapfs, &byte, 1);

    // TODO return errors ( + doc)
}
