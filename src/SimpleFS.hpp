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
    int clearInode(unsigned inode);
    INode getTargetDirectory(const std::vector<std::string> &path);

public:
    explicit SimpleFS(std::string path);
    int create(std::string && path, unsigned short mode);
    int _open(std::string && name, int mode);
    int _read(int fd, char * buf, int len);
    int _write(int fd, char * buf, int len);
    int _lseek(int fd, int whence, int offset);
    int unlink(std::string && name);
    int mkdir(std::string && name);
    int rmdir(std::string && name);
};

#endif
