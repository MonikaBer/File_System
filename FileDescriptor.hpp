//Class representing structure of file descriptor
#ifndef FILE_DESCRIPTOR_HPP
#define FILE_DESCRIPTOR_HPP

class FileDescriptor {
private:
    int inode_id;
    int file_cursor;
    unsigned int type;

public:
    //methods declarations
    int getInodeId() const;

};

#endif