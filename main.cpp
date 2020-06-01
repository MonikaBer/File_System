#include <iostream>
#include "SimpleFS.hpp"

int main() {
    SimpleFS fs = SimpleFS("/home/kulson/misc/etc/simplefs/simplefs.conf");
    fs.createSystemFiles();
    return 0;
}