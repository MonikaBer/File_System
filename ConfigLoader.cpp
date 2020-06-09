#include "ConfigLoader.hpp"
#include <fstream>
#include <sstream>
#include <cmath>

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
    hostStreams[FdNames::BLOCKS_BITMAP].open(getBlocksBitmapPath(), std::ios::binary);
    hostStreams[FdNames::INODES_BITMAP].open(getInodesBitmapPath(), std::ios::binary);
    hostStreams[FdNames::INODES].open(getInodesPath(), std::ios::binary);
    hostStreams[FdNames::BLOCKS].open(getBlocksPath(), std::ios::binary);
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
    return hostStreams[FdNames::BLOCKS_BITMAP];
}

std::fstream & ConfigLoader::getInodesBitmap() const {
    return hostStreams[FdNames::INODES_BITMAP];
}

std::fstream & ConfigLoader::getInodes() const {
    return hostStreams[FdNames::INODES];
}

std::fstream & ConfigLoader::getBlocks() const {
    return hostStreams[FdNames::BLOCKS];
}

int ConfigLoader::getSizeOfInode() const {
    return sizeofInode;
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
    ofs.seekp(getMaxNumberOfInodes()*sizeofInode - 1);
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
    ofs.seekp(getMaxNumberOfBlocks() * sizeOfBlock - 1);
    ofs.write("", 1);
}
