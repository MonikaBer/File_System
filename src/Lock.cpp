#include "Lock.hpp"
#include <stdexcept>
#include "ResourceManager.hpp"

Lock::Lock(Type type, unsigned int inodeId, int fsFileDescriptor, long startPosition, long length):
    type(type),
    inodeId(inodeId),
    fileDescriptor(fsFileDescriptor)
{
    if(type == WR_LOCK)
        fileLockDescription.l_type = F_WRLCK;
    else if(type == RD_LOCK)
        fileLockDescription.l_type = F_RDLCK;
    else
        throw std::runtime_error("Bad lock type.");
    fileLockDescription.l_pid = 0;
    fileLockDescription.l_whence = SEEK_SET;
    fileLockDescription.l_start = startPosition;
    fileLockDescription.l_len = length;
    execute();
}

Lock::Lock(Lock::Type type, unsigned int inodeId):
    Lock(type, inodeId, ResourceManager::getInstance()->getInodes(), inodeId*INode::sizeofInode, INode::sizeofInode){

}

//void Lock::acquire() {
//    execute();
//}

void Lock::release() {
    fileLockDescription.l_type = F_UNLCK;
    execute();
}

void Lock::execute() {
    if (fcntl(fileDescriptor, F_SETLKW, &fileLockDescription) == -1) //Blocking lock
        throw std::runtime_error("Fcntl failed");
}

Lock::~Lock() {
    release();
}
