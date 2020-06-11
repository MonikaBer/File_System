//Class representing structure of file descriptor
#include "FileDescriptor.hpp"

void FileDescriptor::writeToInode(char *buffer, int len) {
    fileCursor += inode->writeToFile(buffer, len, fileCursor);
}
