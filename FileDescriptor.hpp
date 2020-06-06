//Class representing structure of file descriptor
#ifndef FILE_DESCRIPTOR_HPP
#define FILE_DESCRIPTOR_HPP

class FileDescriptor {
private:
    unsigned int inode_id;
    int file_cursor;
    unsigned int type;

public:
    //methods declarations
    FileDescriptor() = default;
    FileDescriptor(unsigned int inodeId, unsigned int type) : inode_id(inodeId), type(type) {}
    unsigned int getInodeId() const;

};

#endif