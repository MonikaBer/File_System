//Main class of file system
#ifndef SIMPLE_FS_HPP
#define SIMPLE_FS_HPP
static const int blockSize = 4096;

#include <string>
#include <vector>
#include <map>

#include "Lock.hpp"
#include "FileDescriptor.hpp"
#include "INode.hpp"


class SimpleFS {
private:
    const static int sizeofInode = sizeof(unsigned short) + sizeof(long) + 14 * sizeof(unsigned);

    enum fdNames {BLOCKS_BITMAP, INODES_BITMAP, INODES, BLOCKS};
    static const int hostFd[4];

    //files paths
    std::string blocksBitmap;
    std::string inodesBitmap;
    std::string inodes;
    std::string blocks;

    int maxNumberOfBlocks;
    int maxNumberOfInodes;

    std::vector<Lock> openInodes;
    std::vector<FileDescriptor*> fds;

    std::vector<std::string> parseDirect(const std::string& path);
    int findFreeInode();
    static int createBitmapFile(const std::string& path, int numberOfBits);
    int createInodesFile(const std::string& path) const;
    int createBlocksFile(const std::string& path) const;
    static inline bool fileExists (const std::string& name);

public: //TODO: Change to private
    int writeInode(FileDescriptor& fd, INode& inode);
    int readInode(FileDescriptor& fd, INode& inode);
    int clearInode(FileDescriptor&fd);
    unsigned getFreeBlock();
    int freeBlock(unsigned block);
    int getTargetDirectoryINode(const std::string& path);

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
