#include <iostream>
#include <fstream>
#include <array>
#include <algorithm>
#include <cstring>

#include "SimpleFS.hpp"

class test {
    std::array<unsigned, 3> blocks = {0};
};

int main() {
    SimpleFS fs = SimpleFS("../etc/simplefs.conf");
    std::shared_ptr<INode> iNode = std::make_shared<INode>(6, 0x100e, 0xeded, 0x1234, 0x5678);
    INode iNode1(8, 0x100e, 0xeded, 0x1234, 0x5678);
    FileDescriptor fd = FileDescriptor(iNode, Lock::Type::RD_LOCK);
    std::cout << "size of iNode:" << sizeof(iNode1) << std::endl;
    std::cout << "size of iNode class:" << sizeof(INode) << std::endl;;
    std::cout << "size of iNode obj:" << iNode1.sizeofInode << std::endl;
    std::cout << "size of test:" << sizeof(test) << std::endl;
    fs.writeInode(fd);
    for (int i = 0; i < 5; i ++) {
        unsigned freeBlock = fs.getFreeBlock();
        std::cout << "Got new free block: " << freeBlock << std::endl;
    }
    char xd[512];
    std::memcpy(&xd, &iNode1, sizeof(iNode));

    return 0;
}