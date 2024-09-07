#include "AConfigBuilder.h"
#include <fstream>
#include <string>

void    AConfigBuilder::InitSettingsVector(std::ifstream& ifs, const std::string& lvl_descrt)
{
    (void)lvl_descrt;
    std::string content;
    while (std::getline(ifs >> std::ws, content)) {
        if (content.empty() || content[0] == '#') {
            continue;
        }
        char    last_char = *content.rbegin();
        content.erase(content.find_last_not_of(" \t" + std::string(1, last_char)) + 1); // utils function trim
        
        if (last_char == ';') {
            if (content.empty()) {
                throw std::invalid_argument("Empty setting.");
            }
            settings_.push_back(MakePair(content));
        } else if (last_char == '{') {
            if (content.empty()) {
                throw std::invalid_argument("Empty block.");
            }
            Setting setting = MakePair(content);
            nested_builders_.push_back(GetBuilderByLvl(GetNestingByToken(setting.first), ifs, setting.second));
        } else if (last_char == '}' && lvl_ != NS_GLOBAL) {
            if (!content.empty()) {
                throw std::invalid_argument("Invalid paranthesis.");
            }
            return;
        } else {
            throw std::invalid_argument("Invalid config file.");
        }
    }
    if (lvl_ != NS_GLOBAL || settings_.empty()) {
        throw std::invalid_argument("Invalid config file.");
    }
}
