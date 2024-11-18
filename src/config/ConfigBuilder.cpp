#include "ConfigBuilder.h"

#include <str_utils.h>

#include "HttpConfig.h"
#include "HttpConfigBuilder.h"

namespace config {

static ConfigBuilder::MxType ParseMxType(const std::string& val)
{
    if (val == "epoll") {
        return c_api::MT_EPOLL;
    } else if (val == "select") {
        return c_api::MT_SELECT;
    } else if (val == "poll") {
        return c_api::MT_POLL;
    }
    throw std::runtime_error("Invalid configuration file: invalid mx_type: " + val);
}

static ConfigBuilder::MxType BuildMxType(const std::vector<std::string>& vals)
{
    if (vals.empty()) {
        return Config::kDefaultMxType();
    } else if (vals.size() > 1) {
        throw std::runtime_error("Invalid configuration file: duplicated use value.");
    }
    return ParseMxType(vals[0]);
}

static Severity ParseErrorLogLevel(const std::string& val)
{
    if (val == "debug") {
        return DEBUG;
    } else if (val == "info") {
        return INFO;
    } else if (val == "warning") {
        return WARNING;
    } else if (val == "error") {
        return ERROR;
    } else if (val == "fatal") {
        return FATAL;
    } else if (val.empty()) {
        return Config::kDefaultErrorLogLevel();
    }
    throw std::runtime_error("Invalid configuration file: invalid error_log level: " + val);
}

static std::pair<std::string, Severity> BuildErrorLog(const std::vector<std::string>& vals)
{
    if (vals.empty()) {
        return std::make_pair(Config::kDefaultErrorLogPath(), Config::kDefaultErrorLogLevel());
    } else if (vals.size() > 1) {
        throw std::runtime_error("Invalid configuration file: duplicated error_log value.");
    }
    std::vector<std::string> val_elements = utils::fs::SplitLine(vals[0]);
    if (val_elements.size() == 1) {
        return std::make_pair(val_elements[0], Config::kDefaultErrorLogLevel());
    } else if (val_elements.size() == 2) {
        return std::make_pair(val_elements[0], ParseErrorLogLevel(val_elements[1]));
    }
    throw std::runtime_error("Invalid configuration file: invalid error_log setting: " + vals[0]);
}

bool ConfigBuilder::IsKeyAllowed(const std::string& key) const
{
    return key == "use" || key == "error_log";
}

bool ConfigBuilder::AreNestingsValid(const ParsedConfig& f) const
{
    if (f.nested_configs().size() != 1) {
        return false;
    }
    for (std::vector<ParsedConfig>::const_iterator it = f.nested_configs().begin();
         it != f.nested_configs().end(); ++it) {
        if (it->nesting_lvl() != "http" || !it->nesting_lvl_descr().empty()) {
            return false;
        }
    }
    return true;
}

Config ConfigBuilder::Build(const ParsedConfig& f,
                            const InheritedSettings& inherited_settings) const
{
    for (std::map<std::string, std::string>::const_iterator it = f.settings().begin();
         it != f.settings().end(); ++it) {
        if (!IsKeyAllowed(it->first)) {
            throw std::runtime_error("Invalid configuration file: invalid key: " + it->first);
        }
    }
    ConfigBuilder::MxType mx_type = BuildMxType(f.FindSetting("use"));
    std::pair<std::string, Severity> error_log = BuildErrorLog(f.FindSetting("error_log"));

    if (!AreNestingsValid(f)) {
        throw std::runtime_error("Invalid configuration file: invalid nesting.");
    }
    HttpConfigBuilder http_builder;
    HttpConfig http_conf = http_builder.Build(f.nested_configs()[0], inherited_settings);
    return Config(mx_type, error_log, http_conf);
}

const Config ConfigBuilder::GetConfigFromConfFile(const std::string& config_path)
{
    if (!utils::fs::CheckFileExtension(config_path, ".conf")) {
        throw std::invalid_argument("Invalid config file suffix.");
    }

    std::ifstream config_file(config_path.c_str());
    if (!config_file.is_open()) {
        throw std::invalid_argument("Couldn't open config file.");
    }

    ParsedConfig parser(config_file);
    ConfigBuilder config_builder;
    return config_builder.Build(parser);
}

}  // namespace config
