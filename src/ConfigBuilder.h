#ifndef WS_CONFIG_PARS_H
#define WS_CONFIG_PARS_H

#include <fstream>
#include <iostream>
#include <stack>
#include <string>
#include <vector>

#include "Config.h"

class ConfigBuilder {
  public:
    ConfigBuilder(const char* config_path);
    std::vector<setting> ProcessFile(std::ifstream& _config_file);
    setting MakePair(const std::string& line);
    void ParseDirective(setting& parsed_setting);
    void ParseNesting(setting& parsed_setting);
    void PrintSettings();
    const Config& Parse();

  private:
    ConfigBuilder();
    Config config_;
    std::stack<std::string> nesting_;
    std::vector<setting> settings_;
};

#endif  // WS_CONFIG_PARS_H
