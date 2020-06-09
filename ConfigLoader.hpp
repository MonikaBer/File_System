#ifndef CONFIGLOADER_HPP
#define CONFIGLOADER_HPP

#include <string>
#include <map>

class ConfigLoader {
    static std::map<std::string, std::string> map;

private:
    void strip(std::string& string, const std::string& characters_to_avoid = "\t\n\v\f\r ");
    void left_strip(std::string& string, const std::string& characters_to_avoid);
    void right_strip(std::string& string, const std::string& characters_to_avoid);

public:
    static void init(std::string path);
    static std::string getBlocksBitmapPath();
    static std::string getInodesBitmapPath();
    static std::string getInodesPath();
    static std::string getBlocksPath();
    static int getMaxNumberOfBlocks();
    static int getMaxNumberOfInodes();
    static int getMaxLengthOfName();
};

#endif
