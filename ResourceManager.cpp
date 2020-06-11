#include "ResourceManager.hpp"
#include <map>
#include <sstream>
#include <cmath>
#include <fcntl.h>
#include <unistd.h>
#include <fstream>

#include "INode.hpp"

std::unique_ptr<ResourceManager> ResourceManager::loader;

ResourceManager::ResourceManager(std::string path)
{
    std::fstream input(path);
    if(!input.is_open())
        throw std::runtime_error("Couldn't open configuration file.");
    std::string line;
    while(std::getline(input, line)){
        std::stringstream sstream(line);
        std::string key;
        std::getline(sstream, key, ':');
        strip(key);
        std::string value;
        std::getline(sstream, value, '\n');
        strip(value);
        map[key] = value;
    }
    processSystemFiles();
}

void ResourceManager::initialize(std::string path){
    loader = std::make_unique<ResourceManager>(path);
}

ResourceManager* ResourceManager::getInstance(){
    return loader.get();
}

void ResourceManager::strip(std::string& string, const std::string& characters_to_avoid){
    left_strip(string, characters_to_avoid);
    right_strip(string, characters_to_avoid);
}

void ResourceManager::left_strip(std::string& string, const std::string& charactersToAvoid){
    string.erase(0, string.find_first_not_of(charactersToAvoid));
}

void ResourceManager::right_strip(std::string& string, const std::string& charactersToAvoid){
    string.erase(string.find_last_not_of(charactersToAvoid)+1, string.size());
}

std::string ResourceManager::getBlocksBitmapPath(){
    return map["blocks bitmap path"];
}

std::string ResourceManager::getInodesBitmapPath(){
    return map["inodes bitmap path"];
}

std::string ResourceManager::getInodesPath(){
    return map.at("inodes path");
}

std::string ResourceManager::getBlocksPath(){
    return map["blocks path"];
}

int ResourceManager::getMaxNumberOfBlocks() {
    return std::stoi(map["max number of blocks"]);
}

int ResourceManager::getMaxNumberOfInodes() {
    return std::stoi(map["max number of inodes"]);
}

int ResourceManager::getMaxLengthOfName() {
    return std::stoi(map["max length of name"]);
}

int ResourceManager::getBlocksBitmap() {
    int fd = hostFd[BLOCKS_BITMAP];
    lseek(fd, 0, 0);
    return fd;
}

int ResourceManager::getInodesBitmap() {
    int fd = hostFd[INODES_BITMAP];
    lseek(fd, 0, 0);
    return fd;
}

int ResourceManager::getInodes() {
    int fd = hostFd[INODES];
    lseek(fd, 0, 0);
    return fd;
}

int ResourceManager::getBlocks() {
    int fd = hostFd[BLOCKS];
    lseek(fd, 0, 0);
    return fd;
}

int ResourceManager::getSizeOfBlock() {
    return sizeOfBlock;
}

/**
 * Checks for existence of 4 essential files by opening them and creates them if all of them don't exist, otherwise throws.
 *
 * WARNING: losing inodes/blocks file renders whole filesystem useless. Creating empty files is only reasonable if
 * filesystem is being created!
 *
 * @return
 */
int ResourceManager::processSystemFiles() {
    // TODO lost bitmaps could be rebuilt if freed block/inode would have its data overwritten with zeroes
    int successfulOpensCounter = 0;
    successfulOpensCounter += openFile(BLOCKS_BITMAP, getBlocksBitmapPath());
    successfulOpensCounter += openFile(INODES_BITMAP, getInodesBitmapPath());
    successfulOpensCounter += openFile(BLOCKS, getBlocksPath());
    successfulOpensCounter += openFile(INODES, getInodesPath());
    if(successfulOpensCounter == 0){
        createFile(BLOCKS_BITMAP, getBlocksBitmapPath(), getMaxNumberOfBlocks());
        createFile(INODES_BITMAP, getInodesBitmapPath(), getMaxNumberOfInodes());
        createFile(BLOCKS, getBlocksPath(), getMaxNumberOfBlocks() * sizeOfBlock - 1);
        createFile(INODES, getInodesPath(), getMaxNumberOfInodes()* INode::sizeofInode - 1);
        INode root(0, 1, 0, 0, 0);
        root.writeInode();
    }
    else if(successfulOpensCounter != 4)
        throw std::runtime_error("There must be all SimpleFS files or none");
}

bool ResourceManager::openFile(FdNames type, std::string path){
    int openFileDescriptor = open(path.data(), O_RDWR);
    if(openFileDescriptor == -1)
        return false;
    hostFd[type] = openFileDescriptor;
    return true;
}

void ResourceManager::createFile(ResourceManager::FdNames type, const std::string &path, int initialSize) {
    int fileDescriptor = open(path.c_str(), O_CREAT | O_RDWR, 0666); //TODO: Write for others not working (mode bug)
    if(fileDescriptor == -1)
        throw std::runtime_error("Cannot create file of path " + path);
    ftruncate(fileDescriptor, initialSize);
    hostFd[type] = fileDescriptor;
}

/**
 * Allows to take ownership of unowned block.
 *
 * @return 0 - no free blocks found
 * @return positive number - block number
 */
unsigned ResourceManager::getFreeBlock() {
    int bitmapfs = ResourceManager::getInstance()->getBlocksBitmap();
    unsigned block = 0;
    unsigned char byte;
    bool looking = true;
    while (looking) {
        read(bitmapfs, (char*)&byte, 1);
        if(byte == EOF)
            throw std::runtime_error("Couldn't find free block");
        if (byte == 255) {    // all blocks taken
            block += 8;
            continue;
        }
        for (int i = 0; i < 8; i++) {
            if ( ((byte >> i) & 0x1) == 0) {
                looking = false;
                break;
            }
            block++;
        }
    }
    byte |= 1 << (block%8);
    lseek(bitmapfs, -1, SEEK_CUR);
    write(bitmapfs, (char*)&byte, 1);
    return block;
}

/**
 * Allows to renounce ownership of block
 *
 * WARNING: data is not erased!
 *
 * @param block - block number
 * @return
 */
int ResourceManager::freeBlock(unsigned int block) {
    // TODO in any way doesnt check who is the owner of this block! - should it be changed?
    // TODO should data be cleared?
    // TODO return values + doc
    if (block == 0) {   // never free root block!
        return -1;
    }
    int bitmapfs = ResourceManager::getInstance()->getBlocksBitmap();
    lseek(bitmapfs, block/8, SEEK_SET);
    char byte;
    read(bitmapfs, &byte, 1);
    byte &= ~(1 << (block%8));
    lseek(bitmapfs, block/8, SEEK_SET);
    write(bitmapfs, &byte, 1);
}

// TODO iNode in indirect block expects value 0 to find its end! either add blockscount, clear block after freeing or clear next 4 bytes after assinging new block