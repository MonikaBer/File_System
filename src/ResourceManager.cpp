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
}

void ResourceManager::initialize(std::string path){
    loader = std::make_unique<ResourceManager>(path);
    loader->processSystemFiles();
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
        createFile(BLOCKS_BITMAP, getBlocksBitmapPath(), getMaxNumberOfBlocks()/8);
        createFile(INODES_BITMAP, getInodesBitmapPath(), getMaxNumberOfInodes()/8);
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