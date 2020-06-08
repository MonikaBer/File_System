//Class representing structure of file lock
#ifndef LOCK_HPP
#define LOCK_HPP

class Lock {
public:
    enum Type {RD_LOCK, WR_LOCK};
private:
    unsigned inode_id;
    Type type;
public:
    Lock(Type type, unsigned inodeId);
};

#endif