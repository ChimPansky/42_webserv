#include "ConfigBuilder.h"
#include "Config.h"

ConfigBuilder::ConfigBuilder(std::ifstream& config_file, const std::string& lvl_descrt)
    : AConfigBuilder(config_file, lvl_descrt)
{
    lvl_ = NS_GLOBAL;
}

ConfigBuilder::~ConfigBuilder()
{}

ConfigBuilder ConfigBuilder::StartBuilder(const std::string& config_path)
{
    if (config_path.length() < 6 || (config_path.find_last_of('.') != std::string::npos && config_path.substr(config_path.find_last_of('.')) != ".conf")) {
        throw std::invalid_argument("Invalid config file suffix.");
    }

    std::ifstream   config_file(config_path.c_str());
    if (!config_file.is_open()) {
        throw std::invalid_argument("Couldn't open config file.");
    }
    return ConfigBuilder(config_file, "");
}

const std::vector<std::string>  ConfigBuilder::GetTokensByLvl() const
{
    std::vector<std::string>    tokens;

    tokens.push_back("use");
    tokens.push_back("error_log");
    return tokens;
}

utils::shared_ptr<IConfig>  ConfigBuilder::Parse()
{
    if (nested_builders_.size() != 1) {
        throw std::invalid_argument("Invalid config file: no http block.");
    }
    return utils::shared_ptr<IConfig>(new Config(settings_, nested_builders_[0]->Parse()));
}
