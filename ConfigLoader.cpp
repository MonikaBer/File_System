#include "ConfigLoader.hpp"
#include <fstream>
#include <sstream>
#include <cmath>
#include <iostream>

std::unique_ptr<ConfigLoader> ConfigLoader::loader;
std::map<std::string, std::string> ConfigLoader::map;
std::fstream ConfigLoader::hostStreams[4];

ConfigLoader::ConfigLoader(std::string path)
{
    std::ifstream input(path);
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
    createSystemFiles();
    hostStreams[FdNames::BLOCKS_BITMAP].open(getBlocksBitmapPath(), std::ios::binary | std::ios::in | std::ios::out);
    hostStreams[FdNames::INODES_BITMAP].open(getInodesBitmapPath(), std::ios::binary | std::ios::in | std::ios::out);
    hostStreams[FdNames::INODES].open(getInodesPath(), std::ios::binary | std::ios::in | std::ios::out);
    hostStreams[FdNames::BLOCKS].open(getBlocksPath(), std::ios::binary | std::ios::in | std::ios::out);
}

void ConfigLoader::init(std::string path) {
    loader = std::make_unique<ConfigLoader>(path);
}

ConfigLoader* ConfigLoader::getInstance(){
    return loader.get();
}

void ConfigLoader::strip(std::string& string, const std::string& characters_to_avoid){
    left_strip(string, characters_to_avoid);
    right_strip(string, characters_to_avoid);
}

void ConfigLoader::left_strip(std::string& string, const std::string& charactersToAvoid){
    string.erase(0, string.find_first_not_of(charactersToAvoid));
}

void ConfigLoader::right_strip(std::string& string, const std::string& charactersToAvoid){
    string.erase(string.find_last_not_of(charactersToAvoid)+1, string.size());
}

std::string ConfigLoader::getBlocksBitmapPath(){
    return map["blocks bitmap path"];
}

std::string ConfigLoader::getInodesBitmapPath(){
    return map["inodes bitmap path"];
}

std::string ConfigLoader::getInodesPath(){
    return map["inodes path"];
}

std::string ConfigLoader::getBlocksPath(){
    return map["blocks path"];
}

int ConfigLoader::getMaxNumberOfBlocks() const {
    return std::stoi(map["max number of blocks"]);
}

int ConfigLoader::getMaxNumberOfInodes() const{
    return std::stoi(map["max number of inodes"]);
}

int ConfigLoader::getMaxLengthOfName() const{
    return std::stoi(map["max length of name"]);
}

std::fstream & ConfigLoader::getBlocksBitmap() const {
    hostStreams[FdNames::BLOCKS_BITMAP].seekg(0);
    hostStreams[FdNames::BLOCKS_BITMAP].seekp(0);
    return hostStreams[FdNames::BLOCKS_BITMAP];
}

std::fstream & ConfigLoader::getInodesBitmap() const {
    hostStreams[FdNames::INODES_BITMAP].seekg(0);
    hostStreams[FdNames::INODES_BITMAP].seekp(0);
    return hostStreams[FdNames::INODES_BITMAP];
}

std::fstream & ConfigLoader::getInodes() const {
    hostStreams[FdNames::INODES].seekg(0);
    hostStreams[FdNames::INODES].seekp(0);
    return hostStreams[FdNames::INODES];
}

std::fstream & ConfigLoader::getBlocks() const {
    hostStreams[FdNames::BLOCKS].seekg(0);
    hostStreams[FdNames::BLOCKS].seekp(0);
    return hostStreams[FdNames::BLOCKS];
}

int ConfigLoader::getSizeOfBlock() const {
    return sizeOfBlock;
}

/**
 * Checks for existence of 4 essential files and creates them if they don't exist.
 *
 * WARNING: losing inodes/blocks file renders whole filesystem useless. Creating empty files is only reasonable if
 * filesystem is being created!
 *
 * @return
 */
int ConfigLoader::createSystemFiles() {
    // TODO name is kinda unfortunate - not always creates, only when needed.
    // TODO creating single file is pointless - losing blocks/inodes renders whole filesystem
    // TODO lost bitmaps could be rebuilt if freed block/inode would have its data overwritten with zeroes
    // blocks bitmap file
    if (!fileExists(getBlocksBitmapPath()))
        createBitmapFile(getBlocksBitmapPath(), getMaxNumberOfBlocks());
    if (!fileExists(getInodesBitmapPath()))
        createBitmapFile(getInodesBitmapPath(), getMaxNumberOfInodes());
    if (!fileExists(getBlocksPath()))
        createBlocksFile(getBlocksPath());
    if (!fileExists(getInodesPath()))
        createInodesFile(getInodesPath());
}

inline bool ConfigLoader::fileExists (const std::string& path) {
    if (FILE *file = fopen(path.c_str(), "r")) {
        fclose(file);
        return true;
    } else {
        return false;
    }
}

int ConfigLoader::createBitmapFile(const std::string &path, int numberOfBits) {
    std::ofstream ofs(path, std::ios::binary);
    if(!ofs.is_open())
        throw std::runtime_error("Cannot open file of path " + path);
    char reserved = 1;
    ofs.write(&reserved, 1);        // reserve bit [0] for root
    ofs.seekp(ceil(numberOfBits/8.0) - 1);
    ofs.write("", 1);
}

/**
 * Creates empty file big enough to hold all INodes of SimpleFS.
 *
 * @param path - path where file will be created
 * @return
 */
int ConfigLoader::createInodesFile(const std::string &path) const {
    std::ofstream ofs(path, std::ios::binary);
    if(!ofs.is_open())
        throw std::runtime_error("Cannot open file of path " + path);
    ofs.seekp(getMaxNumberOfInodes()*INode::sizeofInode - 1);
    ofs.write("", 1);
}

/**
 * Creates empty file big enough to hold all blocks of SimpleFS.
 *
 * @param path - path where file will be created
 * @return
 */
int ConfigLoader::createBlocksFile(const std::string &path) const {
    std::ofstream ofs(path, std::ios::binary);
    if(!ofs.is_open())
        throw std::runtime_error("Cannot open file of path " + path);
    ofs.seekp(getMaxNumberOfBlocks() * sizeOfBlock - 1);
    ofs.write("", 1);
}

/**
 * Allows to take ownership of unowned block.
 *
 * @return 0 - no free blocks found
 * @return positive number - block number
 */
unsigned ConfigLoader::getFreeBlock() {
    std::fstream& bitmapfs = ConfigLoader::getInstance()->getBlocksBitmap();
    unsigned block = 0;
    unsigned char byte;
    bool looking = true;

    while (looking) {
        bitmapfs.read((char*)&byte, 1);
        if (bitmapfs.eof())
            return 0;
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
    long pos = bitmapfs.tellg();
    bitmapfs.seekp(pos - 1);
    bitmapfs.write((char*)&byte, 1);
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
int ConfigLoader::freeBlock(unsigned int block) {
    // TODO in any way doesnt check who is the owner of this block! - should it be changed?
    // TODO should data be cleared?
    // TODO return values + doc
    if (block == 0) {   // never free root block!
        return -1;
    }

    std::fstream& bitmapfs = ConfigLoader::getInstance()->getBlocksBitmap();
    bitmapfs.seekg(block/8);
    char byte;
    bitmapfs.read(&byte, 1);
    byte &= ~(1 << (block%8));
    bitmapfs.seekp(block/8);
    bitmapfs.write(&byte, 1);
}

// TODO iNode in indirect block expects value 0 to find its end! either add blockscount, clear block after freeing or clear next 4 bytes after assinging new block