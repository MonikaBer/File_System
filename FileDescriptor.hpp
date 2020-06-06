//Class representing structure of file descriptor
#ifndef FILE_DESCRIPTOR_HPP
#define FILE_DESCRIPTOR_HPP

class FileDescriptor {
private:
    unsigned int inode_id;
    long file_cursor;
    unsigned int type;  // TODO kinda unfortunate name - seems like file/directory, but instead is type of lock!

public:
    //methods declarations
    FileDescriptor() = default;
    FileDescriptor(unsigned int inodeId, unsigned int type) : inode_id(inodeId), type(type), file_cursor(0) {}

    unsigned int getInodeId() const { return inode_id; }
    long getFileCursor() const { return file_cursor; }
    unsigned int getType() const { return type; }
    void setFileCursor(long fileCursor) { file_cursor = fileCursor; }
};

#endif