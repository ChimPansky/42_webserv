#ifndef WS_CONFIG_PARS_H
#define WS_CONFIG_PARS_H

#include <fstream>
#include <stack>
#include <string>
#include <vector>

#include "Config.h"

class ConfigBuilder {
  public:
    ConfigBuilder(const char* config_path);
    std::vector<setting> ProcessFile(std::ifstream& _config_file);
    setting MakePair(const std::string& line);
    bool HandleParanthesis(setting& setting, const std::string& line);
    // const   std::vector<std::string>  SplitLine(const std::string& line);
    const Config& Parse();
    void ParseDirective(setting parsed_setting);
    void ParseNesting(setting parsed_setting);
    void AppendNestingPath(std::string& token, std::stack<std::string> temp_stack);

  private:
    ConfigBuilder();
    Config config_;
    std::stack<std::string> nesting_;
    std::vector<setting> settings_;
    std::string block_name_;
    typedef std::pair<std::string, std::string> setting;
};

#endif  // WS_CONFIG_PARS_H
