#include <iostream>
#include <fstream>
#include <array>
#include <algorithm>
#include "SimpleFS.hpp"

int main() {
    SimpleFS fs = SimpleFS("/home/kulson/misc/etc/simplefs/simplefs.conf");
    fs.createSystemFiles();
    FileDescriptor fd = FileDescriptor(8, 0);
    INode iNode = INode(0x100e, 0xeded, 0x1234, 0x5678);
    fs.writeInode(fd, iNode);

    return 0;
}