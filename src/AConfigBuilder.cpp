#include "AConfigBuilder.h"
#include "HttpConfigBuilder.h"
#include "ServerConfigBuilder.h"
#include "LocationConfigBuilder.h"

AConfigBuilder::AConfigBuilder(std::ifstream& ifs, const std::string& lvl_descrt)
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

utils::unique_ptr<AConfigBuilder>   AConfigBuilder::GetBuilderByLvl(NestingLevel lvl, std::ifstream& ifs, const std::string& lvl_descrt)
{
    switch (lvl) {
        case NS_HTTP:
            return utils::unique_ptr<AConfigBuilder>(new HttpConfigBuilder(ifs, lvl_descrt));
        case NS_SERVER:
            return utils::unique_ptr<AConfigBuilder>(new ServerConfigBuilder(ifs, lvl_descrt));
        case NS_LOCATION:
            return utils::unique_ptr<AConfigBuilder>(new LocationConfigBuilder(ifs, lvl_descrt));
        default:
            throw std::invalid_argument("Invalid block.");
    }
}

NestingLevel    AConfigBuilder::GetNestingByToken(const std::string& token)
{
    switch (lvl_) {
        case NS_GLOBAL: {
            if (token == "http") {
                return NS_HTTP;
            } else {
                throw std::invalid_argument("Invalid block.");
            }
        }
        case NS_HTTP: {
            if (token == "server") {
                return NS_SERVER;
            } else {
                throw std::invalid_argument("Invalid block.");
            }
        }
        case NS_SERVER: {
            if (token == "location") {
                return NS_LOCATION;
            } else {
                throw std::invalid_argument("Invalid block.");
            }
        }
        default:
            throw std::invalid_argument("Invalid block.");
    }
}

AConfigBuilder::Setting AConfigBuilder::MakePair(const std::string& line)
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
