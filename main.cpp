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
    fs.create("/plik", 1);
    fs.create("/plikDwa", 1);
    int fd = fs.open("/plikDwa", 1);
    std::cout << "fd plikDwa: " << fd;
    fd = fs.open("/plik", 1);
    std::cout << "fd plik: " << fd;

    return 0;
}