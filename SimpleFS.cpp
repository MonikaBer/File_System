//Main class of file system
#include <fstream>
#include <cmath>
#include "SimpleFS.hpp"
#include "ConfigLoader.hpp"

SimpleFS::SimpleFS(std::string && configPath) {
    ConfigLoader loader(configPath);
    blocks_bitmap = loader.getBlocksBitmapPath();
    inodes_bitmap = loader.getInodesBitmapPath();
    inodes = loader.getInodesPath();
    blocks = loader.getBlocksPath();
    max_number_of_blocks = loader.getMaxNumberOfBlocks();
    max_number_of_inodes = loader.getMaxNumberOfInodes();
}


int SimpleFS::create(std::string && name, int mode) {
    std::vector<std::string> parsed_path = parse_direct(name);
    if(parsed_path.empty())
        return -1;
    //lock root;
    for(auto file = parsed_path.begin(); file != parsed_path.end()-1; ++file){
        //lock file;
    }

    return -1;
}


int SimpleFS::open(std::string && name, int mode) {
        return -1;
}


int SimpleFS::read(int fd, char * buf, int len) {
    return -1;
}


int SimpleFS::write(int fd, char * buf, int len) {
    return -1;
}


int SimpleFS::lseek(int fd, int whence, int offset) {
    return -1;
}


int SimpleFS::unlink(std::string && name) {
    return -1;
}


int SimpleFS::mkdir(std::string && name) {
    return -1;
}


int SimpleFS::rmdir(std::string && name) {
    return -1;
}

std::vector<std::string> SimpleFS::parse_direct(std::string & path) {
    std::vector<std::string> parsed_path;
    if(!path.empty() && path[0] == '/') {
        std::string path_element;
        for (auto a = path.begin() + 1; a != path.end(); ++a) {
            if (*a == '/' or a == path.end() - 1) {
                parsed_path.push_back(path_element);
                path_element.clear();
            } else {
                path_element.push_back(*a);
            }
        }
    }
    return parsed_path;
}

int SimpleFS::find_free_inode() {
    std::ifstream input(inodes_bitmap);
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

int SimpleFS::createSystemFiles() {
    // blocks bitmap file
    if(!fileExists(blocks_bitmap)) {
        createBitmapFile(blocks_bitmap, max_number_of_blocks);
    }
    if(!fileExists(inodes_bitmap)) {
        createBitmapFile(inodes_bitmap, max_number_of_inodes);
    }
    if(!fileExists(blocks)) {
        createBlocksFile(blocks);
    }
    if(!fileExists(inodes)) {
        createInodesFile(inodes);
    }
}

inline bool SimpleFS::fileExists (const std::string& path) {
    if (FILE *file = fopen(path.c_str(), "r")) {
        fclose(file);
        return true;
    } else {
        return false;
    }
}

int SimpleFS::createBitmapFile(const std::string &path, int numberOfBits) {
    std::ofstream ofs(path, std::ios::binary | std::ios::out);
    ofs.seekp(ceil(numberOfBits/8.0) - 1);
    ofs.write("", 1);
}

int SimpleFS::createInodesFile(const std::string &path) {
    std::ofstream ofs(path, std::ios::binary | std::ios::out);
    ofs.seekp(max_number_of_inodes*sizeofInode - 1);
    ofs.write("", 1);
}

int SimpleFS::createBlocksFile(const std::string &path) {
    std::ofstream ofs(path, std::ios::binary | std::ios::out);
    ofs.seekp(max_number_of_blocks*4096 - 1);
    ofs.write("", 1);
}
