#include "ConfigLoader.hpp"
#include <fstream>
#include <sstream>

ConfigLoader::ConfigLoader(std::string path)
{
    std::ifstream input(path);
    if(!input.is_open())
        throw std::runtime_error("Couldn't open configuration file.");
    std::string line;
    while(std::getline(input, line)){
        std::stringstream sstream(line);
        std::string key;
        std::getline(sstream, key, ':');
        strip(key);
        std::string value;
        std::getline(sstream, value, '\n');
        strip(value);
        map[key] = value;
    }
}

void ConfigLoader::strip(std::string& string, const std::string& characters_to_avoid){
    left_strip(string, characters_to_avoid);
    right_strip(string, characters_to_avoid);
}

void ConfigLoader::left_strip(std::string& string, const std::string& charactersToAvoid){
    string.erase(0, string.find_first_not_of(charactersToAvoid));
}

void ConfigLoader::right_strip(std::string& string, const std::string& charactersToAvoid){
    string.erase(string.find_last_not_of(charactersToAvoid)+1, string.size());
}

std::string ConfigLoader::getBlocksBitmapPath(){
    return map["blocks bitmap path"];
}

std::string ConfigLoader::getInodesBitmapPath(){
    return map["inodes bitmap path"];
}

std::string ConfigLoader::getInodesPath(){
    return map["inodes path"];
}

std::string ConfigLoader::getBlocksPath(){
    return map["blocks path"];
}

int ConfigLoader::getMaxNumberOfBlocks(){
    return std::stoi(map["max number of blocks"]);
}

int ConfigLoader::getMaxNumberOfInodes(){
    return std::stoi(map["max number of inodes"]);
}

int ConfigLoader::getMaxLengthOfName(){
    return std::stoi(map["max length of name"]);
}