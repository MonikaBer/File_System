#include <iostream>
#include <cstring>

#include "simplefs.h"

void testWrite() {
    simplefs_creat("/test", 1);
    simplefs_creat("/testDwa", 1);
    simplefs_creat("/testTrzy", 1);
    int fd = simplefs_open("/testDwa", 1);
    std::cout << "fd testDwa: " << fd << std::endl;
    fd = simplefs_open("/test", 1);
    std::cout << "fd test: " << fd << std::endl;
    fd = simplefs_open("/testTrzy", 1);
    std::cout << "fd testTrzy: " << fd << std::endl;
    char *text = "WIELKI TEST";
    char read_buff[11];
    simplefs_write(2, text, 11);
    simplefs_lseek(2, 0, 0);
    simplefs_read(2, read_buff, 11);
    std::cout << read_buff << std::endl;
}

void testWrongPath(){
    create("/dir", 1);
    std::cout<<create("/dir/abc/test", 0);
    std::cout<<open("/dir/abc/test", 1);
}

int main() {
    testWrongPath();
    return 0;
}