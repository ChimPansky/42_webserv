#include "ConfigBuilder.h"

ConfigParser::ConfigParser(std::ifstream& ifs, const std::string& lvl, const std::string& lvl_descr)
    : lvl_(lvl),
      lvl_descr_(lvl_descr)
{
    std::string content;
    while (std::getline(ifs >> std::ws, content)) {
        if (content.empty() || content[0] == '#') {
            continue;
        }
        char    last_char = *content.rbegin();  // TEST
        content.erase(content.find_last_not_of(" \t" + std::string(1, last_char)) + 1); // utils function trim
        
        // TEST and improve
        if (last_char == ';') {
            if (content.empty()) {
                throw std::invalid_argument("Empty setting.");
            }
            settings_.insert(MakePair(content));
        } else if (last_char == '{') {
            if (content.empty()) {
                throw std::invalid_argument("Empty block.");
            }
            Setting setting = MakePair(content);
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

std::vector<std::string> ConfigParser::FindSetting(const std::string& key) const {

    std::vector<std::string> res;
    for (std::multimap<std::string, std::string>::const_iterator it = settings_.begin(); it != settings_.end(); ++it) {
        if (it->first == key) {
            res.push_back(it->second);
        }
    }
    return res;
}

const ConfigParser&    ConfigParser::FindNesting(const std::string& key, int idx) const {

    if (nested_configs_.empty()) {
        throw std::runtime_error("Invalid configuration file: no " + key + " block.");
    } else if ("http" == key && nested_configs_.size() == 1 && "http" == nested_configs_[0].lvl()) {
        return nested_configs_[idx];
    } else if ("server" == key || "location" == key) {
        if (key != nested_configs_[0].lvl()) {
            throw std::runtime_error("Invalid configuration file: invalid block.");
        }
        return nested_configs_[idx];
    }
    throw std::runtime_error("Invalid configuration file: multiple " + key + " blocks.");
}

ConfigParser::Setting ConfigParser::MakePair(const std::string& line)
{
    size_t  pos = line.find_first_of(" \t");
    if (pos == std::string::npos) {
        throw std::invalid_argument("Invalid configuration file: no value specified.");
    }

    size_t  start = line.find_first_not_of(" \t", pos);
    if (start == std::string::npos) {
        throw std::invalid_argument("Invalid configuration file: no value specified.");
    }
    return std::make_pair(line.substr(0, pos), line.substr(start, line.size() - start));
}

const Config    ConfigParser::GetConfig(const std::string& config_path) // maybe do all three: parse, build and return valid Config
{
    if (config_path.length() < 6 || (config_path.find_last_of('.') != std::string::npos && config_path.substr(config_path.find_last_of('.')) != ".conf")) {
        throw std::invalid_argument("Invalid config file suffix.");
    }

    std::ifstream   config_file(config_path.c_str());
    if (!config_file.is_open()) {
        throw std::invalid_argument("Couldn't open config file.");
    }
    ConfigParser    parser(config_file, "", "");

    return ConfigBuilder<Config>::Build(parser); 
}
