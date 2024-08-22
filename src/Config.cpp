#include "Config.h"

void    process_file(const char* config_path) {

    std::ifstream   infile;

    infile.open(config_path);
    if (!infile.is_open())
        throw std::runtime_error("Could not open configuration file.");
    
    
    std::string content;
    while (std::getline(infile, content)) {

    }
}

Config::Config(const char* config_path) {

}
