#include "Lock.hpp"

Lock::Lock(Type type, unsigned int inodeId) {
    this->type = type;
    this->inode_id = inodeId;
}