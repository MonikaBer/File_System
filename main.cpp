#include <iostream>
#include <cstring>

#include "SimpleFS.hpp"
#include "simplefs.h"

//void showLseek() {
//    SimpleFS fs = SimpleFS("../etc/simplefs.conf");
//    fs.create("/test", 1);
//    fs.create("/testDwa", 1);
//    fs.create("/testTrzy", 1);
//    int fd = fs.open("/testDwa", 1);
//    std::cout << "fd testDwa: " << fd << std::endl;
//    fd = fs.open("/test", 1);
//    std::cout << "fd test: " << fd << std::endl;
//    fd = fs.open("/testTrzy", 1);
//    std::cout << "fd testTrzy: " << fd << std::endl;
//    std::cout << "Opened files: " << std::endl;
//    for (FileDescriptor fDesc : fs.getFds()) {
//        std::shared_ptr<INode> inode = fDesc.getInode();
//        std::cout << "Inode: " << inode->getId() << "Cursor: " << fDesc.getFileCursor() << std::endl;
//    }
//    fs.lseek(0, 0, 30);
//    fs.lseek(1, 0, 30);
//    fs.lseek(1, 1, 30);
//    fs.lseek(2, 2, 20); // TODO na razie plik ma zerowa dlugosc wiec to nie pyknie
//    for (FileDescriptor fDesc : fs.getFds()) {
//        std::shared_ptr<INode> inode = fDesc.getInode();
//        std::cout << "Inode: " << inode->getId() << "Cursor: " << fDesc.getFileCursor() << std::endl;
//    }
//}
//
void testWrite() {
    create("/test", 1);
    create("/testDwa", 1);
    create("/testTrzy", 1);
    int fd = open("/testDwa", 1);
    std::cout << "fd testDwa: " << fd << std::endl;
    fd = open("/test", 1);
    std::cout << "fd test: " << fd << std::endl;
    fd = open("/testTrzy", 1);
    std::cout << "fd testTrzy: " << fd << std::endl;
    char *text = "WIELKI TEST";
    char read_buff[38];
    write(2, text, 11 + 1);
    write(2, text, 11 + 1);
    lseek(2, 0, 0);
    read(2, read_buff, 38+1);
    std::cout << read << std::endl;
}

int main() {
    testWrite();
//    create("/test", 1);
    return 0;
}