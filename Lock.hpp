//Class representing structure of file lock
#ifndef LOCK_HPP
#define LOCK_HPP

#include <fcntl.h>

class Lock {
public:
    enum Type {RD_LOCK, WR_LOCK};
private:
    unsigned inodeId;
    Type type;
    struct flock fileLockDescription;
    int fileDescriptor;

    void execute();
public:
    Lock(Type type, unsigned inodeId, int fsFileDescriptor, long startPosition, long length);
    void release();
};

#endif