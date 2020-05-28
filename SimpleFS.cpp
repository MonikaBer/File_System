//Main class of file system
#include "SimpleFS.hpp"
#include <fstream>
#include "ConfigLoader.hpp"

SimpleFS::SimpleFS(std::string && configPath) {
    ConfigLoader loader(configPath);
    blocks_bitmap = loader.getBlocksBitmapPath();
    inodes_bitmap = loader.getInodesBitmapPath();
    inodes = loader.getInodesPath();
    blocks = loader.getBlocksPath();
    max_number_of_blocks = loader.getMaxNumberOfBlocks();
    max_number_of_inodes = loader.getMaxNumberOfInodes();
}


int SimpleFS::create(std::string && name, int mode) {
    return -1;
}


int SimpleFS::open(std::string && name, int mode) {
    return -1;
}


int SimpleFS::read(int fd, char * buf, int len) {
    return -1;
}


int SimpleFS::write(int fd, char * buf, int len) {
    return -1;
}


int SimpleFS::lseek(int fd, int whence, int offset) {
    return -1;
}


int SimpleFS::unlink(std::string && name) {
    return -1;
}


int SimpleFS::mkdir(std::string && name) {
    return -1;
}


int SimpleFS::rmdir(std::string && name) {
    return -1;
}