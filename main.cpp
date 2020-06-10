#include <iostream>
#include <cstring>

#include "SimpleFS.hpp"
#include "simplefs.h"

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
    char read_buff[11];
    write(2, text, 11);
    lseek(2, 0, 0);
    read(2, read_buff, 11);
    std::cout << read_buff << std::endl;
}

int main() {
    testWrite();
    return 0;
}