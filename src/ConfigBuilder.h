#ifndef WS_CONFIG_PARS_H
#define WS_CONFIG_PARS_H

#include <fstream>
#include <iostream>
#include <stack>
#include <string>
#include <vector>

#include "Config.h"

class ConfigBuilder {
  private:
    typedef std::pair<std::string, std::string> S;

  public:
    ConfigBuilder(const std::string& config_path);
    std::vector<S> ProcessFile(std::ifstream& _config_file);
    S MakePair(const std::string& line);
    void ParseDirective(S& parsed_setting, std::stack<std::string>& nesting_level);
    void ParseNesting(S& parsed_setting, std::stack<std::string>& nesting_level);
    void PrintSettings();
    static const std::vector<std::string> GetTokensByLevel(const std::string& lvl);
    const Config Parse();

  private:
    ConfigBuilder();
    std::vector<S> settings_;
};

#endif  // WS_CONFIG_PARS_H
