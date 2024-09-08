#include "ServerConfigBuilder.h"
#include "LocationConfig.h"
#include "ServerConfig.h"

ServerConfigBuilder::ServerConfigBuilder(std::ifstream& config_file, const std::string& lvl_descrt)
    : AConfigBuilder(config_file, lvl_descrt)
{
    lvl_ = NS_SERVER;
}

ServerConfigBuilder::~ServerConfigBuilder()
{}

const std::vector<std::string>  ServerConfigBuilder::GetTokensByLvl() const
{
    std::vector<std::string>    tokens;

    tokens.push_back("listen");
    tokens.push_back("server_name");
    tokens.push_back("access_log");
    tokens.push_back("error_log");
    tokens.push_back("root");
    tokens.push_back("index");
    tokens.push_back("autoindex");
    return tokens;
}

utils::shared_ptr<IConfig>  ServerConfigBuilder::Parse()
{
    std::vector<utils::shared_ptr<LocationConfig> > location_configs;

    if (nested_builders_.size() < 1) {
        throw std::invalid_argument("Invalid config file: no http block.");
    }
    for(size_t i = 0; i < nested_builders_.size(); i++) {
        location_configs.push_back(nested_builders_[i]->Parse());
    }
    return utils::shared_ptr<IConfig>(new ServerConfig(settings_, location_configs));
}
