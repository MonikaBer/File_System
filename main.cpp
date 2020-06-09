#include <iostream>
#include <fstream>
#include <array>
#include <algorithm>
#include "SimpleFS.hpp"

int main() {
    SimpleFS fs = SimpleFS("../etc/simplefs.conf");
    fs.createSystemFiles();
    FileDescriptor fd = FileDescriptor(8, 0);
    INode iNode = INode(0x100e, 0xeded, 0x1234, 0x5678);
    fs.writeInode(fd, iNode);
    for (int i = 0; i < 5; i ++) {
        unsigned freeBlock = fs.getFreeBlock();
        std::cout << "Got new free block: " << freeBlock << std::endl;
    }
    return 0;
}