#include "conf_pars.h"
#include <iostream>
#include <sstream>
#include <stack>
#include <string>
#include <utility>

namespace config_pars {

bool    ValidSettingPair(std::pair<std::string, std::string>& setting, const std::string& line)
{
    if (line.find('{') != std::string::npos && line.find(';') != std::string::npos) {
        return false;
    } else if (line.find(';') != std::string::npos) {
        if (setting.second[setting.second.size() - 1] != ';') {
            return false;
        }
    } else if (line.find('{') != std::string::npos) {
        if ((setting.second != "{" && setting.first != "location") || setting.second[setting.second.size() - 1] != '{' || (setting.first != "server" && setting.first != "location" && setting.first != "http")) {
            return false;
        }
    }
    return true;
}

std::pair<std::string, std::string> MakeSettingPair(const std::string& line, std::stack<std::string> nesting)
{
    std::pair<std::string, std::string> setting;
    std::istringstream ss(line);
    std::string setting_path;

    ss >> setting.first;
    std::getline(ss, setting.second); //    TODO: remove spaces
    while (!nesting.empty()) {
        if (nesting.top() != "{") {
            if (nesting.top() != "location" && nesting.top() != "server" && nesting.top() != "http") {
                throw std::runtime_error("Invalid configuration file: invalid context");
            } else {
                setting_path = nesting.top() + ":" + setting.second;
            }
        nesting.pop();
        }
    }
    if (setting.first.empty() || setting.second.empty() || !ValidSettingPair(setting, line)) {
        throw std::runtime_error("Invalid configuration file: invalid format");
    }
    if (!setting_path.empty()) {
        setting.first = setting_path + ":" + setting.first;
    }
    return setting;
}


void    AddPair(std::map<std::string, std::string>& map, const std::pair<std::string, std::string>& setting)
{
    if (map.find(setting.first) != map.end()) { // TODO : check if i need to check for duplicates
        throw std::runtime_error("Duplicate setting in configuration file");
    }
    map.insert(setting);
}

const std::vector<std::string>  SplitLine(const std::string& line)
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
}

void    printMap(const std::map<std::string, std::string>& vec)
{
    std::cout << "Line: [";
    for (std::map<std::string, std::string>::const_iterator it = vec.begin(); it != vec.end(); it++) {
        std::cout << it->first << ":" << it->second;
        if (it != vec.end()) {
            std::cout << "|";
        }
    }
    std::cout << "] \n";
}

std::map<std::string, std::string>    ProcessFile(std::ifstream& _config_file)
{
    std::stack<std::string> paranthesis;
    std::pair<std::string, std::string> setting;
    std::map<std::string, std::string> settings;
    std::string block;

    std::string content;
    while (std::getline(_config_file >> std::ws, content)) {
        if (content.empty() || content[0] == '#') { // TODO: handle comments after settings
            continue;
        } else if (content.find_first_of(";{") != std::string::npos) {
            setting = MakeSettingPair(content, paranthesis);
            AddPair(settings, setting);
            if (content.find('{') != std::string::npos) {
                paranthesis.push(setting.first);
                paranthesis.push("{");
            }    //parse block
        } else if (content.find('}') != std::string::npos) {
            if (paranthesis.empty() || paranthesis.top() != "{" || content != "}") { // check if i have spaces after paranthesis
                throw std::runtime_error("Invalid configuration file: invalid paranthesis");
            }
            paranthesis.pop();
            paranthesis.pop();
        } else {
            throw std::runtime_error("Invalid configuration file: invalid content" + content);
        }
    }
    if (!paranthesis.empty()) {
        throw std::runtime_error("Configuration file is empty");
    }
    return settings;
}

} // namespace config_pars

