#include "HttpConfigBuilder.h"
#include "HttpConfig.h"
#include "ServerConfig.h"

HttpConfigBuilder::HttpConfigBuilder(std::ifstream& config_file, const std::string& lvl_descrt)
    : AConfigBuilder(config_file, lvl_descrt)
{
    lvl_ = NS_HTTP;
}

HttpConfigBuilder::~HttpConfigBuilder()
{}

const std::vector<std::string>  HttpConfigBuilder::GetTokensByLvl() const
{
    std::vector<std::string>    tokens;

    tokens.push_back("keepalive_timeout");
    tokens.push_back("client_max_body_size");
    tokens.push_back("error_page");
    tokens.push_back("root");
    tokens.push_back("index");
    tokens.push_back("autoindex");
    return tokens;
}

utils::shared_ptr<IConfig>  HttpConfigBuilder::Parse()
{
    std::vector<utils::shared_ptr<ServerConfig> > server_configs;

    if (nested_builders_.size() < 1) {
        throw std::invalid_argument("Invalid config file: no http block.");
    }
    for(size_t i = 0; i < nested_builders_.size(); i++) {
        server_configs.push_back(nested_builders_[i]->Parse());
    }
    return utils::shared_ptr<IConfig>(new HttpConfig(settings_, server_configs));
}
