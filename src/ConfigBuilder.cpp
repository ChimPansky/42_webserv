#include "ConfigBuilder.h"
#include "AConfigBuilder.h"
#include <fstream>
#include <string>

ConfigBuilder::ConfigBuilder(std::ifstream& config_file)
{
    InitSettingsVector(config_file, "");
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
    return ConfigBuilder(config_file);
}

const std::vector<std::string>  ConfigBuilder::GetTokensByLvl() const
{
    std::vector<std::string>    tokens;

    tokens.push_back("use");
    tokens.push_back("error_log");
    return tokens;
}

NestingLevel    ConfigGetNestingByToken(const std::string& token)
{
    if (token == "http") {
        return NS_HTTP;
    } else {
        throw std::invalid_argument("Invalid block.");
    }
}

utils::unique_ptr<IConfigBuilder>  ConfigBuilder::GetBuilderByLvl(NestingLevel lvl)
{}

utils::unique_ptr<IConfig>  ConfigBuilder::Parse() const
{

}
