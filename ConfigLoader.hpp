#ifndef CONFIGLOADER_HPP
#define CONFIGLOADER_HPP

#include <string>
#include <map>

class ConfigLoader {
    std::map<std::string, std::string> map;

private:
    void strip(std::string& string, const std::string& characters_to_avoid = "\t\n\v\f\r ");
    void left_strip(std::string& string, const std::string& characters_to_avoid);
    void right_strip(std::string& string, const std::string& characters_to_avoid);

public:
    ConfigLoader(std::string path);
    std::string getBlocksBitmapPath();
    std::string getInodesBitmapPath();
    std::string getInodesPath(); 
    std::string getBlocksPath();
    int getMaxNumberOfBlocks(); 
    int getMaxNumberOfInodes();
};

#endif