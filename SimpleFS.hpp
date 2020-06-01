//Main class of file system
#ifndef SIMPLE_FS_HPP
#define SIMPLE_FS_HPP
#include <string>
#include <vector>

#include "Lock.hpp"
#include "FileDescriptor.hpp"

class SimpleFS {
private:
    const int sizeofInode = sizeof(unsigned short) + sizeof(long) + 14 * sizeof(unsigned);

    //files paths
    std::string blocks_bitmap;
    std::string inodes_bitmap;
    std::string inodes; 
    std::string blocks;
        
    int max_number_of_blocks; 
    int max_number_of_inodes;

    std::vector<Lock> open_files;
    std::vector<FileDescriptor*> fds;

    std::vector<std::string> parse_direct(std::string & path);
    int find_free_inode();
    int createBitmapFile(const std::string& path, int numberOfBits);
    int createInodesFile(const std::string& path);
    int createBlocksFile(const std::string& path);

    inline bool fileExists (const std::string& name);
public:
    SimpleFS(std::string && configPath);
    int create(std::string && name, int mode);
    int open(std::string && name, int mode);
    int read(int fd, char * buf, int len);
    int write(int fd, char * buf, int len);
    int lseek(int fd, int whence, int offset);
    int unlink(std::string && name);
    int mkdir(std::string && name);
    int rmdir(std::string && name);
    int createSystemFiles();
};

#endif
