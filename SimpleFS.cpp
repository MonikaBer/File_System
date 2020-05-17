//Main class of file system
#include "SimpleFS.hpp"


SimpleFS::SimpleFS(std::string && config) {

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