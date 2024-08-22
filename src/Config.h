#ifndef WS_CONFIG_H
#define WS_CONFIG_H

#include <iostream>
#include <fstream>
#include <map>

// hopefully will be trivially-copyable
class Config {
  private:
    Config();
    std::map<std::string, std::string> _configValues;
  public:
    Config(const char* config_path);
};

#endif  // WS_CONFIG_H
