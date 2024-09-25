#include "ConfigParser.h"
#include <iostream>

namespace config {

ConfigParser::ConfigParser(std::ifstream& ifs, const std::string& lvl, const std::string& lvl_descr)
    : lvl_(lvl), lvl_descr_(lvl_descr)
{
    std::string content;
    while (std::getline(ifs >> std::ws, content)) {
        if (content.empty() || content[0] == '#') {
            continue;
        }
        content = Trim(content, " \t");
        char last_char = *content.rbegin();
        content = Trim(content, " \t" + std::string(1, last_char));

        // TEST and improve
        if (last_char == ';') {
            if (content.empty()) {
                throw std::invalid_argument("Empty setting.");
            }
            settings_.insert(config::MakePair(content));
        } else if (last_char == '{') {
            if (content.empty()) {
                throw std::invalid_argument("Empty block.");
            }
            Setting setting = config::MakePair(content);
            nested_configs_.push_back(ConfigParser(ifs, setting.first, setting.second));
        } else if (last_char == '}' && !lvl_.empty()) {
            if (!content.empty()) {
                throw std::invalid_argument("Invalid paranthesis.");
            }
            return;
        } else {
            throw std::invalid_argument("Invalid config file.");
        }
    }
    if (!lvl_.empty()) {
        throw std::invalid_argument("Invalid config file.");
    }
}

const std::string& ConfigParser::lvl() const
{
    return lvl_;
}

const std::string& ConfigParser::lvl_descr() const
{
    return lvl_descr_;
}

const std::multimap<std::string, std::string>& ConfigParser::settings() const
{
    return settings_;
}
const std::vector<ConfigParser>& ConfigParser::nested_configs() const
{
    return nested_configs_;
}

std::vector<std::string> ConfigParser::FindSetting(const std::string& key) const
{
    std::vector<std::string> res;
    for (std::multimap<std::string, std::string>::const_iterator it = settings_.begin();
         it != settings_.end(); ++it) {
        if (it->first == key) {
            res.push_back(it->second);
        }
    }
    return res;
}

const std::vector<ConfigParser>& ConfigParser::FindNesting(const std::string& key) const
{
    if (nested_configs_.empty()) {
        throw std::runtime_error("Invalid configuration file: no " + key + " block.");
    } else if ("http" == key && nested_configs_.size() == 1 && "http" == nested_configs_[0].lvl()) {
        return nested_configs_;
    } else if ("server" == key || "location" == key) {
        for (size_t i = 0; i < nested_configs_.size(); i++) {
            if (key != nested_configs_[i].lvl()) {
                throw std::runtime_error("Invalid configuration file: invalid block.");
            }
        }
        return nested_configs_;
    }
    throw std::runtime_error("Invalid configuration file: multiple " + key + " blocks.");
}

}  // namespace config
