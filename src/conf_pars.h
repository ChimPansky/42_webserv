#ifndef WS_CONFIG_PARS_H
#define WS_CONFIG_PARS_H

#include <iostream>
#include <fstream>
#include <map>
#include <stack>
#include <string>
#include <vector>

namespace config_pars {

    bool    HandleParanthesis(std::pair<std::string, std::string>& setting, const std::string& line);
    std::pair<std::string, std::string> MakeSettingPair(const std::string& line, const std::string& block);
    //const   std::vector<std::string>  SplitLine(const std::string& line);
    std::vector<std::pair<std::string, std::string> >    ProcessFile(std::ifstream& _config_file);
} // namespace config_pars

#endif  // WS_CONFIG_PARS_H
