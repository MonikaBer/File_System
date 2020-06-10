//Main class of file system
#include <fstream>
#include <cmath>
#include "SimpleFS.hpp"
#include "ConfigLoader.hpp"
#include <functional>
#include "Lock.hpp"

SimpleFS::SimpleFS(std::string && configPath) {
    ConfigLoader::init(configPath);
}


// TODO nie pozwol stworzyc plikow jesli maja taka sama nazwe
int SimpleFS::create(std::string && path, unsigned short mode) {
    std::vector<std::string> parsedPath = parseDirect(path);
    if(parsedPath.empty())
        return -1;
    std::string newFileName = parsedPath.back();
    parsedPath.pop_back();
    INode targetDirINode;
    try{
        targetDirINode = getTargetDirectory(parsedPath);
    }catch(...){
        return -1;
    }
    int freeInodeId = findFreeInode();
    if(freeInodeId < 0)
        return -1;
    INode newFile = INode(freeInodeId, mode, 0, 0, 0); //todo numberofblocks and indirectblock ????
    targetDirINode.saveINodeInDirectory(newFileName, newFile);
    // todo locks xdd
    return 0;
}


int SimpleFS::open(std::string && path, int mode) {
    std::vector<std::string> parsedPath = parseDirect(path);
    if(parsedPath.empty())
        return -1;
    std::string fileName = parsedPath.back();
    parsedPath.pop_back();
    try{
        INode targetDirINode = getTargetDirectory(parsedPath);
        std::map<std::string, unsigned> dirContent = targetDirINode.getDirectoryContent();
        std::shared_ptr<INode> openINode = std::make_shared<INode>(dirContent[fileName]);

        if (mode == 1)
            fds.emplace_back(openINode, Lock::WR_LOCK);
        else
            fds.emplace_back(openINode, Lock::RD_LOCK);

    } catch(std::runtime_error& e){
        std::cout << e.what();
        return -1;
    }

    return fds.size()-1;
}


int SimpleFS::read(int fd, char * buf, int len) {
    // TODO not tested!
    if ( fd >= fds.size() )
        return -1;

    FileDescriptor &fdr = fds[fd];
    unsigned cursor = fdr.getFileCursor();
    std::shared_ptr<INode> inode = fdr.getInode();
    unsigned totalRead = 0;

    std::fstream& blocksFile = ConfigLoader::getInstance()->getBlocks();

    // if trying to read more than left in file, cut len so it stops on end of file
    if (len + cursor > inode->getLength())
        len = inode->getLength() - cursor;

    int blockSize = ConfigLoader::getInstance()->getSizeOfBlock();
    while (len) {
        // calculate host file offset
        unsigned iNodeBlockIndex = cursor / blockSize;
        unsigned blockOffset = cursor % blockSize;
        unsigned block = inode->getBlock(iNodeBlockIndex);
        unsigned long host_file_offset = block * blockSize + blockOffset;

        unsigned singleRead = len;
        // if this read spans over more than 1 block, read must be divided into parts
        if (blockOffset + len > blockSize)
            singleRead = blockSize - blockOffset;

        blocksFile.seekg(host_file_offset);
        blocksFile.read(buf+totalRead, singleRead);

        totalRead += singleRead;
        cursor += singleRead;
        len -= singleRead;
    }

    return totalRead;
}


int SimpleFS::write(int fd, char * buf, int len) {
    FileDescriptor descriptor = fds[fd];
    descriptor.writeToInode(buf, len);
}


int SimpleFS::lseek(int fd, int whence, int offset) {
    // TODO not tested!
    if ( fd >= fds.size() )
        return 0; // TODO what to return if error? both positive and negative numbers are taken!

    FileDescriptor &fdr = fds[fd];
    std::shared_ptr<INode> inode = fdr.getInode();

    if (whence == 0) {
        if (offset < 0 || offset >= inode->getLength()) {
            return 0; // TODO what to return if error? both positive and negative numbers are taken!
        }
        fdr.setFileCursor(offset);
    }
    else if (whence == 1) {
        long cursor = fdr.getFileCursor();
        if (offset + cursor < 0 || offset + cursor >= inode->getLength()) {
            return 0; // TODO what to return if error? both positive and negative numbers are taken!
        }
        fdr.setFileCursor(offset + cursor);
    }
    else if (whence == 2) {
        if (offset > 0 || offset + inode->getLength() < 0) {
            return 0; // TODO what to return if error? both positive and negative numbers are taken!
        }
        // - 1 because length points to byte AFTER last byte of file, which is against lseek description in documentation
        fdr.setFileCursor(inode->getLength() - 1 + offset);
    }
    else return 0; // TODO what to return if error? both positive and negative numbers are taken!
    return offset;
}


int SimpleFS::unlink(std::string && name) {
    std::vector<std::string> parsedPath = parseDirect(name);
    INode targetDirINode;
    if(parsedPath.empty())
        return -1;
    std::string fileToDelete = parsedPath.back();
    parsedPath.pop_back();
    try{
        targetDirINode = getTargetDirectory(parsedPath);
    }catch(...){
        return -1;
    }
    std::map<std::string, unsigned> dirContent = targetDirINode.getDirectoryContent();
    unsigned iNodeToDelete = dirContent[fileToDelete]; // TODO LOCKI
    INode inode(iNodeToDelete);
    inode.freeAllBlocks();
    clearInode(inode.getId()); // TODO locks
    return -1;
}


int SimpleFS::mkdir(std::string && path) {
    std::vector<std::string> parsedPath = parseDirect(path);
    if(parsedPath.empty())
        return -1;
    std::string newDirName = parsedPath.back();         //name of new directory
    parsedPath.pop_back();
    INode targetDirINode;
    try {
        targetDirINode = getTargetDirectory(parsedPath);    //parent directory for new directory
    } catch (...) {
        return -1;
    }
    int freeInodeId = findFreeInode();
    if(freeInodeId < 0)
        return -1;
    INode newDir = INode(freeInodeId, 1, 0, 0, 0); //todo numberofblocks and indirectblock ????
    targetDirINode.save(newDirName, newDir);

    return -1;
}


int SimpleFS::rmdir(std::string && name) {


    return -1;
}

INode SimpleFS::getTargetDirectory(const std::vector<std::string> &path) {
    INode inode = INode(0);
    for(const auto & fileName : path){
        std::map<std::string, unsigned> dir = inode.getDirectoryContent();
        inode = INode(dir[fileName]);
    }
    return inode;
}

std::vector<std::string> SimpleFS::parseDirect(const std::string& path) {
    if(path.empty() || path[0] != '/')
        return {};
    std::vector<std::string> parsed_path;
    std::string path_element;
    for (auto a = path.begin() + 1; a != path.end(); ++a) {
        if (*a == '/') {
            parsed_path.push_back(path_element);
            path_element.clear();
            continue;
        }
        path_element.push_back(*a);
        if (a == path.end() - 1 ) {
            parsed_path.push_back(path_element);
        }
    }
    return parsed_path;
}

int SimpleFS::findFreeInode() {
    std::fstream &input = ConfigLoader::getInstance()->getInodesBitmap();
    if(!input.is_open())
        throw std::runtime_error("Couldn't open inodes bitmap file.");

    std::string line( (std::istreambuf_iterator<char>(input)),(std::istreambuf_iterator<char>()));
    if(line.empty())
        return -1;

    std::size_t free_inode_byte = line.find_first_not_of(0xFF);
    if(free_inode_byte == std::string::npos)
        return -1;

    char byteWithFreeINode = line[free_inode_byte];
    unsigned int id = 0;
    while((byteWithFreeINode>>id) & 0x1)
        ++id;
    byteWithFreeINode |= 1<<id;
    input.seekp(free_inode_byte);
    input.write((char*)&byteWithFreeINode, 1);

    return 8*free_inode_byte+id;
}


/**
 * Clears INode by clearing corresponding bit in bitmap. Data is not cleared.
 *
 * @param fd - file descriptor holding INode number of file to delete
 * @return 
 */
int SimpleFS::clearInode(FileDescriptor &fd) {
    std::fstream& bitmapfs = ConfigLoader::getInstance()->getInodesBitmap();
    bitmapfs.seekg(fd.getInode()->getId()/8);
    char byte;
    bitmapfs.read(&byte, 1);
    byte &= ~(1 << (fd.getInode()->getId()%8));
    bitmapfs.seekp(fd.getInode()->getId()/8);
    bitmapfs.write(&byte, 1);
    
    // TODO return errors ( + doc)
}

int SimpleFS::clearInode(unsigned inode) {
    std::fstream& bitmapfs = ConfigLoader::getInstance()->getInodesBitmap();
    bitmapfs.seekg(inode/8);
    char byte;
    bitmapfs.read(&byte, 1);
    byte &= ~(1 << (inode%8));
    bitmapfs.seekp(inode/8);
    bitmapfs.write(&byte, 1);

    // TODO return errors ( + doc)
}

