#ifndef SIMPLEFS_H
#define SIMPLEFS_H

extern "C" {
    int create(char *name, int mode);
    int open(char *name, int mode);
    int read(int fd, char *buf, int len);
    int write(int fd, char *buf, int len);
    int lseek(int fd, int whence, int offset);
    int unlink(char *name);
    int mkdir(char *name);
    int rmdir(char *name);
}

#endif