#include <iostream>
#include <fstream>
#include "SimpleFS.hpp"

int main() {
    SimpleFS fs = SimpleFS("/home/kulson/misc/etc/simplefs/simplefs.conf");
    fs.createSystemFiles();
    INode iNode(0xafaf, 0xa1b2c3d4e5f6a2b3, 0x1234, 0x67676767);
    iNode.addBlock(0x1111);
    iNode.addBlock(0x2222);
    iNode.addBlock(0x3333);
//    iNode.addBlock(0x4444);
//    std::ofstream ofs("/home/kulson/misc/etc/simplefs/test", std::ios::binary);
//    ofs << iNode;
    std::ifstream ifs("/home/kulson/misc/etc/simplefs/test", std::ios::binary);
    INode i;
    std::cout << ifs.good();
    ifs >> i;
    return 0;
}