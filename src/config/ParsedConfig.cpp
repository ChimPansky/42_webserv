#include "ParsedConfig.h"

namespace config {

ParsedConfig::ParsedConfig(std::ifstream& ifs, const std::string& nesting_lvl, const std::string& nesting_lvl_descr)
    : nesting_lvl_(nesting_lvl), nesting_lvl_descr_(nesting_lvl_descr)
{
    std::string content;
    while (std::getline(ifs >> std::ws, content)) {
        if (content.empty() || content[0] == '#') {
            continue;
        }
        content = Trim(content, " \t");
        char last_char = *content.rbegin();
        content = Trim(content, " \t" + std::string(1, last_char));

        if (last_char == ';') {
            if (content.empty()) {
                throw std::invalid_argument("Invalid config file: empty setting.");
            }
            settings_.insert(config::MakePair(content));
        } else if (last_char == '{') {
            if (content.empty()) {
                throw std::invalid_argument("Invalid config file: empty block.");
            }
            Setting setting = config::MakePair(content);
            nested_configs_.push_back(ParsedConfig(ifs, setting.first, setting.second));
        } else if (last_char == '}' && !nesting_lvl_.empty()) {
            if (!content.empty()) {
                throw std::invalid_argument("Invalid config file: invalid paranthesis.");
            }
            return;
        } else {
            throw std::invalid_argument("Invalid config file.");
        }
    }
    if (!nesting_lvl_.empty()) {
        throw std::invalid_argument("Invalid config file.");
    }
}

const std::string& ParsedConfig::nesting_lvl() const
{
    return nesting_lvl_;
}

const std::string& ParsedConfig::nesting_lvl_descr() const
{
    return nesting_lvl_descr_;
}

const std::multimap<std::string, std::string>& ParsedConfig::settings() const
{
    return settings_;
}

const std::vector<ParsedConfig>& ParsedConfig::nested_configs() const
{
    return nested_configs_;
}

std::vector<std::string> ParsedConfig::FindSetting(const std::string& key) const
{
    std::vector<std::string> res;
    typedef std::multimap<std::string, std::string>::const_iterator SettingsIt;

    std::pair<SettingsIt, SettingsIt> settings_range = settings_.equal_range(key);

    for (std::multimap<std::string, std::string>::const_iterator it = settings_range.first; it != settings_range.second; ++it) {
        res.push_back(it->second);
    }
    return res;
}

const std::vector<ParsedConfig>& ParsedConfig::FindNesting(const std::string& key) const // maybe this function isn't needed at all
{
    if (nested_configs_.empty()) {
        throw std::runtime_error("Invalid configuration file: no " + key + " block.");
    }
    for (size_t i = 0; i < nested_configs_.size(); i++) {
        if (key != nested_configs_[i].nesting_lvl()) {
            throw std::runtime_error("Invalid configuration file: invalid block.");
        }
    }
    return nested_configs_;
}

}  // namespace config
