#ifndef SIMPLEFS_H
#define SIMPLEFS_H

//extern "C" {
    int simplefs_creat(char *name, int mode);
    int simplefs_open(char *name, int mode);
    int simplefs_read(int fd, char *buf, int len);
    int simplefs_write(int fd, char *buf, int len);
    int simplefs_lseek(int fd, int whence, int offset);
    int simplefs_unlink(char *name);
    int simplefs_mkdir(char *name);
    int simplefs_rmdir(char *name);
//}

#endif