//Class representing structure of file descriptor
#include "FileDescriptor.hpp"

int FileDescriptor::writeToInode(char *buffer, int len) {
    int writtenData = inode->writeToFile(buffer, len, fileCursor);
    fileCursor += writtenData;
    return writtenData;
}
