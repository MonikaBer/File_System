//Main class of file system
#include <fstream>
#include <cmath>
#include "SimpleFS.hpp"
#include "ConfigLoader.hpp"

SimpleFS::SimpleFS(std::string && configPath) {
    ConfigLoader::init(configPath);
}

// /aaa/ccs/asda
//int SimpleFS::create(std::string && path, unsigned short mode) {
//    std::vector<std::string> parsedPath = parseDirect(path);
//    if(parsedPath.empty())
//        return -1;
//    std::string newFileName = parsedPath.back();
//    parsedPath.pop_back();
//    try{
//        INode targetDirINode = getTargetDirectory(parsedPath);
//    }catch(...){
//        return -1;
//    }
//    INode newFile = INode(findFreeInode(), mode, 0, 0, 0); //todo numberofblocks and indirectblock ????
//    targetDirINode.save(newFile);
//    // todo locks xdd
//    return 0;
//}


int SimpleFS::open(std::string && name, int mode) {
        return -1;
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
    return -1;
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


int SimpleFS::mkdir(std::string && name) {
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
        if (*a == '/' or a == path.end() - 1) {
            parsed_path.push_back(path_element);
            path_element.clear();
        } else
            path_element.push_back(*a);
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

    unsigned int id = 0;
    while((line[free_inode_byte]<<id) & 0x80)
        ++id;

    input.close();
    return 8*free_inode_byte+id;
}

/**
 * Saves INode to file and sets bit in bitmap to inform that INode is in use.
 *
 * @param fd - file descriptor holding INode number of file to save
 * @param inode - object to save
 * @return
 */
int SimpleFS::writeInode(FileDescriptor &fd) {
    ConfigLoader* loader = ConfigLoader::getInstance();
    std::fstream &ofs = loader->getInodes();
    ofs.seekp(fd.getInode()->getId()*66);
    ofs << fd.getInode();

    // update bitmap
    // TODO maybe split into updateInode (wont change bitmap) and createInode, which will update bitmap
    std::fstream &inodesBitmap = loader->getInodesBitmap();
    inodesBitmap.seekg(fd.getInode()->getId()/8);
    char byte;
    inodesBitmap.read(&byte, 1);
    byte |= 1 << (fd.getInode()->getId()%8);
    inodesBitmap.seekp(fd.getInode()->getId()/8);
    inodesBitmap.write(&byte, 1);

    // TODO return errors ( + doc)
}

/**
 * Reads INode into inode.
 *
 * @param fd - file descriptor holding INode number of file to read
 * @param inode - object to read INode into
 * @return
 */
INode SimpleFS::readInode(FileDescriptor &fd) {
    ConfigLoader* loader = ConfigLoader::getInstance();
    std::fstream& inodes = loader->getInodes();
    inodes.seekg(fd.getInode()->getId()*66);
    inodes >> fd.getInode();

    // T ODO return errors ( + doc)
}

std::shared_ptr<INode> SimpleFS::readInode(int inodeNumber) {
    std::shared_ptr<INode> inode = std::make_shared<INode>();
    ConfigLoader *loader = ConfigLoader::getInstance();
    std::fstream &ifs = loader->getInodes();
    ifs.seekg(inodeNumber*66);
    ifs >> inode;
    return inode;
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

