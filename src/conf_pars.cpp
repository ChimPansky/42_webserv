#include "conf_pars.h"
#include <iostream>
#include <sstream>
#include <stack>
#include <string>
#include <utility>

namespace config_pars {

std::pair<std::string, std::string> MakeSettingPair(const std::string& line, std::stack<std::string> nesting)
{
    std::string setting_path;
    std::pair<std::string, std::string> setting;

    size_t pos = line.find_first_of(" \t");
    if (pos == std::string::npos) {
        throw std::runtime_error("Invalid configuration file: no value specified.");
    }
    std::string keyword = line.substr(0, pos);

    size_t start = line.find_first_not_of(" \t", pos);
    size_t end = line.find_last_not_of(" \t", pos);
    if (start == std::string::npos || end == std::string::npos) {
        throw std::runtime_error("Invalid configuration file: no value specified.");
    }
    std::string value = line.substr(start, end - start);

    while (!nesting.empty()) {
        setting_path += nesting.top() + ":";
        nesting.pop();
    }
    setting = std::make_pair(keyword, value);
    if (!HandleParanthesis(setting, line)) { // add { handling with the stack before the return
        throw std::runtime_error("Invalid configuration file: invalid format");
    }
    if (!setting_path.empty()) {
        keyword = setting_path + keyword;
    }
    setting = std::make_pair(keyword, value);
    return setting;
}

bool    HandleParanthesis(std::pair<std::string, std::string>& setting, const std::string& line)
{
    if (setting.first.find_first_of("{;") != std::string::npos) {
        return false;
    } else if (line.find(';') != std::string::npos && setting.second[setting.second.size() - 1] != ';') {
        return false;
    } else if (line.find('{') != std::string::npos) {
        if (setting.second[setting.second.size() - 1] != '{') {
            return false;
        } else if (setting.first != "location" && setting.second != "{") {
            return false;
        } else if (setting.first != "http" && setting.first != "server" && setting.first != "location") {
            return false;
        }
    }
    return true;
}

/*const std::vector<std::string>  SplitLine(const std::string& line)
{
    std::vector<std::string> setting;
    std::istringstream ss(line);

    std::string word;
    while (ss >> word) {
        setting.push_back(word);
    }
    if (setting.size() < 2) {
        throw std::runtime_error("Invalid configuration file: setting with no value found");
    }
    return setting;
}*/

std::vector<std::pair<std::string, std::string> >    ProcessFile(std::ifstream& _config_file)
{
    std::stack<std::string> nesting_levels;
    std::pair<std::string, std::string> setting;
    std::vector<std::pair<std::string, std::string> > settings;
    std::string block;

    std::string content;
    while (std::getline(_config_file >> std::ws, content)) {
        if (content.empty() || content[0] == '#') { // TODO: handle comments after settings
            continue;
        } else if (content.find_first_of(";{") != std::string::npos) {
            setting = MakeSettingPair(content, nesting_levels);
            if (setting.second[setting.second.size() - 1] == '{') {
                nesting_levels.push(setting.first);
            }
            // TODO : check if duplicates override the value or are they invalid
            settings.push_back(setting);
        } else if (content == "}" && !nesting_levels.empty()) { // check if i have spaces after paranthesis
            nesting_levels.pop();
        } else {
            throw std::runtime_error("Invalid configuration file: invalid content: " + content);
        }
    }
    if (!nesting_levels.empty()) {
        throw std::runtime_error("Configuration file is empty");
    }
    return settings;
}

} // namespace config_pars

