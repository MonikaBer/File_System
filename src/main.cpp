#include <iostream>
#include <cstring>

#include "simplefs.h"

void testWrite() {
    simplefs_creat((char*)"/test", 1);
    simplefs_creat((char*)"/testDwa", 1);
    simplefs_creat((char*)"/testTrzy", 1);
    int fd = simplefs_open((char*)"/testDwa", 1);
    std::cout << "fd testDwa: " << fd << std::endl;
    fd = simplefs_open((char*)"/test", 1);
    std::cout << "fd test: " << fd << std::endl;
    fd = simplefs_open((char*)"/testTrzy", 1);
    std::cout << "fd testTrzy: " << fd << std::endl;
    char *text = {"WIELKI TEST"};
    char read_buff[11];
    simplefs_write(2, text, 11);
    simplefs_lseek(2, 0, 0);
    simplefs_read(2, read_buff, 11);
    std::cout << read_buff << std::endl;
}


int main() {
    std::cout<<simplefs_creat("/new_file", 0)<<std::endl;
    std::cout<<simplefs_creat("/new_file", 0)<<std::endl;
    std::cout<<simplefs_unlink("/new_file")<<std::endl;
    std::cout<<simplefs_creat("/new_file", 0)<<std::endl;

//    testWrongPath();
    return 0;
}