#include <iostream>
#include <fstream>
#include <array>
#include <algorithm>
#include "SimpleFS.hpp"

int main() {
    SimpleFS fs = SimpleFS("../etc/simplefs.conf");
    std::shared_ptr<INode> iNode = std::make_shared<INode>(8, 0x100e, 0xeded, 0x1234, 0x5678);
    FileDescriptor fd = FileDescriptor(iNode, Lock::Type::RD_LOCK);
    fs.writeInode(fd);
    for (int i = 0; i < 5; i ++) {
        unsigned freeBlock = fs.getFreeBlock();
        std::cout << "Got new free block: " << freeBlock << std::endl;
    }
    return 0;
}