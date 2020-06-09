//Main class of file system
#include <fstream>
#include <cmath>
#include "SimpleFS.hpp"
#include "ConfigLoader.hpp"

SimpleFS::SimpleFS(std::string && configPath) {
    ConfigLoader::init(configPath);
}


int SimpleFS::create(std::string && name, int mode) {
    std::vector<std::string> parsed_path = parseDirect(name);


    return -1;
}


int SimpleFS::open(std::string && name, int mode) {
        return -1;
}


int SimpleFS::read(int fd, char * buf, int len) {
    // TODO not tested!
    // TODO possibly add returning EOF when cursor == inode.len + update doc
    if ( fd >= fds.size() )
        return -1;

    FileDescriptor &fdr = *(fds[fd]);
    unsigned cursor = fdr.getFileCursor();
    INode inode = readInode(fdr, inode);  // TODO will have return error implemented
    unsigned totalRead = 0;

    // if trying to read more than left in file, cut len so it stops on end of file
    if (len + cursor > inode.getLength())
        len = inode.getLength() - cursor;

    while (len) {
        // calculate host file offset
        unsigned iNodeBlockIndex = cursor / blockSize;
        unsigned blockOffset = cursor % blockSize;
        unsigned block = inode.getBlock(iNodeBlockIndex);
        unsigned host_file_offset = block * blockSize + blockOffset;

        unsigned singleRead = len;
        // if this read spans over more than 1 block, read must be divided into parts
        if (blockOffset + len > blockSize)
            singleRead = blockSize - blockOffset;

        // TODO read from host file

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

    FileDescriptor &fdr = *(fds[fd]);
    INode inode;
    readInode(fdr, inode);  // TODO will have return error implemented

    if (whence == 0) {
        if (offset < 0 || offset >= inode.getLength()) {
            return 0; // TODO what to return if error? both positive and negative numbers are taken!
        }
        fdr.setFileCursor(offset);
    }
    else if (whence == 1) {
        long cursor = fdr.getFileCursor();
        if (offset + cursor < 0 || offset + cursor >= inode.getLength()) {
            return 0; // TODO what to return if error? both positive and negative numbers are taken!
        }
        fdr.setFileCursor(offset + cursor);
    }
    else if (whence == 2) {
        if (offset > 0 || offset + inode.getLength() < 0) {
            return 0; // TODO what to return if error? both positive and negative numbers are taken!
        }
        // - 1 because length points to byte AFTER last byte of file, which is against lseek description in documentation
        fdr.setFileCursor(inode.getLength() - 1 + offset);
    }
    else return 0; // TODO what to return if error? both positive and negative numbers are taken!
    return offset;
}


int SimpleFS::unlink(std::string && name) {
    // TODO implementation not finished
    // TODO need some way to access inode


    return -1;
}


int SimpleFS::mkdir(std::string && name) {
    return -1;
}


int SimpleFS::rmdir(std::string && name) {
    return -1;
}

Directory SimpleFS::getTargetDirectory(const std::string& path) {
    std::vector<std::string> parsedPath = parseDirect(path);
    unsigned currentDirectoryInode = 0;

    openInodes.emplace_back(Lock::Type::RD_LOCK, currentDirectoryInode);
    for(auto fileName = parsedPath.begin(); fileName != parsedPath.end()-1; fileName++){

    }
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
    std::ifstream input(inodesBitmap);
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
 * Checks for existence of 4 essential files and creates them if they don't exist.
 *
 * WARNING: losing inodes/blocks file renders whole filesystem useless. Creating empty files is only reasonable if
 * filesystem is being created!
 *
 * @return
 */
int SimpleFS::createSystemFiles() {
    // TODO name is kinda unfortunate - not always creates, only when needed.
    // TODO creating single file is pointless - losing blocks/inodes renders whole filesystem
    // TODO lost bitmaps could be rebuilt if freed block/inode would have its data overwritten with zeroes
    // blocks bitmap file
    if(!fileExists(blocksBitmap)) {
        createBitmapFile(blocksBitmap, maxNumberOfBlocks);
    }
    if(!fileExists(inodesBitmap)) {
        createBitmapFile(inodesBitmap, maxNumberOfInodes);
    }
    if(!fileExists(blocks)) {
        createBlocksFile(blocks);
    }
    if(!fileExists(inodes)) {
        createInodesFile(inodes);
    }

    // TODO return errors ( + doc)
}

/**
 * Creates empty file big enough to hold all bits.
 * Can be used to create both INodes and Blocks bitmaps.
 *
 * @param path - path where file will be created
 * @param numberOfBits - number of bit that will be used
 * @return
 */
int SimpleFS::createBitmapFile(const std::string &path, int numberOfBits) {
    std::ofstream ofs(path, std::ios::binary);
    char reserved = 1;
    ofs.write(&reserved, 1);        // reserve bit [0] for root
    ofs.seekp(ceil(numberOfBits/8.0) - 1);
    ofs.write("", 1);

    // TODO return errors ( + doc)
}

/**
 * Creates empty file big enough to hold all INodes of SimpleFS.
 *
 * @param path - path where file will be created
 * @return
 */
int SimpleFS::createInodesFile(const std::string &path) const {
    std::ofstream ofs(path, std::ios::binary);
    ofs.seekp(maxNumberOfInodes*sizeofInode - 1);
    ofs.write("", 1);

    // TODO return errors ( + doc)
}

/**
 * Creates empty file big enough to hold all blocks of SimpleFS.
 *
 * @param path - path where file will be created
 * @return
 */
int SimpleFS::createBlocksFile(const std::string &path) const {
    std::ofstream ofs(path, std::ios::binary);
    ofs.seekp(maxNumberOfBlocks * blockSize - 1);
    ofs.write("", 1);

    // TODO return errors ( + doc)
}

/**
 * Checks for existence of file in host filesystem.
 *
 * @param path - path to file
 * @return true - file exists
 * @return false - file does not exist
 */
inline bool SimpleFS::fileExists (const std::string& path) {
    if (FILE *file = fopen(path.c_str(), "r")) {
        fclose(file);
        return true;
    } else {
        return false;
    }
}

/**
 * Saves INode to file and sets bit in bitmap to inform that INode is in use.
 *
 * @param fd - file descriptor holding INode number of file to save
 * @param inode - object to save
 * @return
 */
int SimpleFS::writeInode(FileDescriptor &fd, INode &inode) {
    std::ofstream ofs(inodes, std::ios::binary | std::ios::in); // ios::in is needed to avoid overwriting whole file!
    ofs.seekp(fd.getInodeId()*sizeofInode);
    ofs << inode;

    // update bitmap
    // TODO maybe split into updateInode (wont change bitmap) and createInode, which will update bitmap
    std::fstream bitmapfs(inodesBitmap, std::ios::binary | std::ios::in | std::ios::out);
    bitmapfs.seekg(fd.getInodeId()/8);
    char byte;
    bitmapfs.read(&byte, 1);
    byte |= 1 << (fd.getInodeId()%8);
    bitmapfs.seekp(fd.getInodeId()/8);
    bitmapfs.write(&byte, 1);

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
    std::ifstream ifs(inodes, std::ios::binary);
    ifs.seekg(fd.getInodeId()*sizeofInode);
    ifs >> inode;

    // TODO return errors ( + doc)
}

INode SimpleFS::readInode(int inodeNumber) {
    std::ifstream ifs(inodes, std::ios::binary);
    ifs.seekg(inodeNumber*sizeofInode);
    ifs >> inode;

    // TODO return errors ( + doc)
}

/**
 * Clears INode by clearing corresponding bit in bitmap. Data is not cleared.
 *
 * @param fd - file descriptor holding INode number of file to delete
 * @return 
 */
int SimpleFS::clearInode(FileDescriptor &fd) {
    std::fstream bitmapfs(inodesBitmap, std::ios::binary | std::ios::in | std::ios::out);
    bitmapfs.seekg(fd.getInodeId()/8);
    char byte;
    bitmapfs.read(&byte, 1);
    byte &= ~(1 << (fd.getInodeId()%8));
    bitmapfs.seekp(fd.getInodeId()/8);
    bitmapfs.write(&byte, 1);
    
    // TODO return errors ( + doc)
}

/**
 * Allows to take ownership of unowned block.
 *
 * @return 0 - no free blocks found
 * @return positive number - block number
 */
unsigned SimpleFS::getFreeBlock() {
    std::fstream bitmapfs(blocksBitmap, std::ios::binary | std::ios::in | std::ios::out);
    unsigned block = 0;
    unsigned char byte;
    bool looking = true;

    while (looking) {
        bitmapfs.read((char*)&byte, 1);
        if (bitmapfs.eof())
            return 0;
        if (byte == 255) {    // all blocks taken
            block += 8;
            continue;
        }
        for (int i = 0; i < 8; i++) {
            if ( ((byte >> i) & 0x1) == 0) {
                looking = false;
                break;
            }
            block++;
        }
    }
    byte |= 1 << (block%8);
    long pos = bitmapfs.tellg();
    bitmapfs.seekp(pos - 1);
    bitmapfs.write((char*)&byte, 1);
    return block;
}

/**
 * Allows to renounce ownership of block
 *
 * WARNING: data is not erased!
 *
 * @param block - block number
 * @return
 */
int SimpleFS::freeBlock(unsigned int block) {
    // TODO in any way doesnt check who is the owner of this block! - should it be changed?
    // TODO should data be cleared?
    // TODO return values + doc
    if (block == 0) {   // never free root block!
        return -1;
    }

    std::fstream bitmapfs(blocksBitmap, std::ios::binary | std::ios::in | std::ios::out);
    bitmapfs.seekg(block/8);
    char byte;
    bitmapfs.read(&byte, 1);
    byte &= ~(1 << (block%8));
    bitmapfs.seekp(block/8);
    bitmapfs.write(&byte, 1);
}

// TODO iNode in indirect block expects value 0 to find its end! either add blockscount, clear block after freeing or clear next 4 bytes after assinging new block