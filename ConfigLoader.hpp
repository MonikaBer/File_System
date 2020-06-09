#ifndef CONFIGLOADER_HPP
#define CONFIGLOADER_HPP

#include <string>
#include <map>
#include <memory>

class ConfigLoader {
    static std::unique_ptr<ConfigLoader> loader;

    static std::map<std::string, std::string> map;

    enum FdNames {BLOCKS_BITMAP=0, INODES_BITMAP=1, INODES=2, BLOCKS=3};
    static std::fstream hostStreams[4];


    static const int sizeOfBlock = 4096;

private:
    void strip(std::string& string, const std::string& characters_to_avoid = "\t\n\v\f\r ");
    void left_strip(std::string& string, const std::string& characters_to_avoid);
    void right_strip(std::string& string, const std::string& characters_to_avoid);

    int createSystemFiles();
    int createBitmapFile(const std::string& path, int numberOfBits);
    int createInodesFile(const std::string& path) const;
    int createBlocksFile(const std::string& path) const;
    inline bool fileExists (const std::string& name);

    std::string getBlocksBitmapPath();
    std::string getInodesBitmapPath();
    std::string getInodesPath();
    std::string getBlocksPath();
public:
    static void init(std::string path);
    static ConfigLoader* getInstance();

    explicit ConfigLoader(std::string path);
    std::fstream & getBlocksBitmap() const;
    std::fstream & getInodesBitmap() const;
    std::fstream & getInodes() const;
    std::fstream & getBlocks() const;
    int getMaxNumberOfBlocks() const;
    int getMaxNumberOfInodes() const;
    int getMaxLengthOfName() const;
    int getSizeOfBlock() const;

};

#endif
