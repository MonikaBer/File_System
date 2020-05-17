//Main class of file system
#ifndef SIMPLE_FS_HPP
#define SIMPLE_FS_HPP
#include <string>
#include <vector>

#include "Lock.hpp"
#include "FileDescriptor.hpp"

class SimpleFS {
private:
    //files paths
    std::string blocks_bitmap;
    std::string inodes_bitmap;
    std::string inodes; 
    std::string blocks;
        
    int max_number_of_blocks; 
    int max_number_of_inodes;

    std::vector<Lock> open_files;
    std::vector<FileDescriptor*> fds;

public:
    SimpleFS(std::string && config);
    int create(std::string && name, int mode);
    int open(std::string && name, int mode);
    int read(int fd, char * buf, int len);
    int write(int fd, char * buf, int len);
    int lseek(int fd, int whence, int offset);
    int unlink(std::string && name);
    int mkdir(std::string && name);
    int rmdir(std::string && name);
};

#endif
