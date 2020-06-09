//Main class of file system
#ifndef SIMPLE_FS_HPP
#define SIMPLE_FS_HPP

#include <string>
#include <vector>
#include <map>
#include <stack>

#include "Lock.hpp"
#include "FileDescriptor.hpp"
#include "INode.hpp"


class SimpleFS {
private:
    std::stack<Lock> openInodes;
    std::vector<FileDescriptor> fds;

    std::vector<std::string> parseDirect(const std::string& path);
    int findFreeInode();
public: //TODO: Change to private
    int writeInode(FileDescriptor& fd);
    //INode readInode(FileDescriptor& fd);
    //INode readInode(int inodeNumber);
    int clearInode(FileDescriptor&fd);
    unsigned getFreeBlock();
    int freeBlock(unsigned block);
    INode getTargetDirectory(const std::vector<std::string> &path);

public:
    explicit SimpleFS(std::string && configPath);
    int create(std::string && path, unsigned short mode);
    int open(std::string && name, int mode);
    int read(int fd, char * buf, int len);
    int write(int fd, char * buf, int len);
    int lseek(int fd, int whence, int offset);
    int unlink(std::string && name);
    int mkdir(std::string && name);
    int rmdir(std::string && name);
};

#endif
