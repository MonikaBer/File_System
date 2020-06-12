#include "SimpleFS.hpp"
#include "simplefs.h"

static SimpleFS simpleFS = SimpleFS("../etc/simplefs.conf");

//extern "C" {
    int simplefs_creat(char * name, int mode) {
        return simpleFS.create(name, mode);
    }

    int simplefs_open(char * name, int mode) {
        return simpleFS._open(name, mode);
    }

    int simplefs_read(int fd, char * buf, int len) {
       return simpleFS._read(fd, buf, len);
    }

    int simplefs_write(int fd, char * buf, int len) {
        return simpleFS._write(fd, buf, len);
    }

    int simplefs_lseek(int fd, int whence, int offset) {
        return simpleFS._lseek(fd, whence, offset);
    }

    int simplefs_unlink(char * name) {
        return simpleFS.unlink(name);
    }

    int simplefs_mkdir(char * name) {
        return simpleFS.mkdir(name);
    }

    int simplefs_rmdir(char * name) {
        return simpleFS.rmdir(name);
    }
//}