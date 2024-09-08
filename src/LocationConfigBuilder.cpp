#include "LocationConfigBuilder.h"
#include "AConfigBuilder.h"
#include "LocationConfig.h"

LocationConfigBuilder::LocationConfigBuilder(std::ifstream& config_file, const std::string& lvl_descrt)
    : AConfigBuilder(config_file, lvl_descrt)
{
    lvl_ = NS_LOCATION;
}

LocationConfigBuilder::~LocationConfigBuilder()
{}

const std::vector<std::string>  LocationConfigBuilder::GetTokensByLvl() const
{
    std::vector<std::string>    tokens;

    tokens.push_back("allow_methods");
    tokens.push_back("return");
    tokens.push_back("cgi_path");
    tokens.push_back("cgi_extension");
    tokens.push_back("root");
    tokens.push_back("index");
    tokens.push_back("autoindex");
    return tokens;
}

utils::shared_ptr<IConfig>  LocationConfigBuilder::Parse()
{
    if (!nested_builders_.empty()) {
        throw std::invalid_argument("Invalid config file: blocks in location context are invalid.");
    }
    return utils::shared_ptr<IConfig>(new LocationConfig(settings_, lvl_descrt_));
}
