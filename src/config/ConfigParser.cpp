#include "ConfigParser.h"

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

ConfigParser    ConfigParser::MakeParser(const std::string& config_path) // maybe do all three: parse, build and return valid Config
{
    if (config_path.length() < 6 || (config_path.find_last_of('.') != std::string::npos && config_path.substr(config_path.find_last_of('.')) != ".conf")) {
        throw std::invalid_argument("Invalid config file suffix.");
    }

    std::ifstream   config_file(config_path.c_str());
    if (!config_file.is_open()) {
        throw std::invalid_argument("Couldn't open config file.");
    }
    return ConfigParser(config_file, "", "");
}

// utils::unique_ptr<ConfigParser>   ConfigParser::GetBuilderByLvl(NestingLevel lvl, std::ifstream& ifs, const std::string& lvl_descrt)
// {
//     switch (lvl) {
//         case NS_HTTP:
//             return utils::unique_ptr<ConfigParser>(new HttpConfigBuilder(ifs, lvl_descrt));
//         case NS_SERVER:
//             return utils::unique_ptr<ConfigParser>(new ServerConfigBuilder(ifs, lvl_descrt));
//         case NS_LOCATION:
//             return utils::unique_ptr<ConfigParser>(new LocationConfigBuilder(ifs, lvl_descrt));
//         default:
//             throw std::invalid_argument("Invalid block.");
//     }
// }

// NestingLevel    ConfigParser::GetNestingByToken(const std::string& token)
// {
//     switch (lvl_) {
//         case NS_GLOBAL: {
//             if (token == "http") {
//                 return NS_HTTP;
//             } else {
//                 throw std::invalid_argument("Invalid block.");
//             }
//         }
//         case NS_HTTP: {
//             if (token == "server") {
//                 return NS_SERVER;
//             } else {
//                 throw std::invalid_argument("Invalid block.");
//             }
//         }
//         case NS_SERVER: {
//             if (token == "location") {
//                 return NS_LOCATION;
//             } else {
//                 throw std::invalid_argument("Invalid block.");
//             }
//         }
//         default:
//             throw std::invalid_argument("Invalid block.");
//     }
// }

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
