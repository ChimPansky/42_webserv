#include "Config.h"

#include "ParsedConfig.h"

namespace config {

Config::Config(MxType mx_type, const std::pair<std::string, Severity>& error_log,
               const HttpConfig& http_config)
    : mx_type_(mx_type), error_log_path_(error_log.first), error_log_level_(error_log.second),
      http_config_(http_config)
{
    this->Print();
}

Config::MxType Config::mx_type() const
{
    return mx_type_;
}

const std::string& Config::error_log_path() const
{
    return error_log_path_;
}

Severity Config::error_log_level() const
{
    return error_log_level_;
}

const HttpConfig& Config::http_config() const
{
    return http_config_;
}

const Config Config::GetConfig(const std::string& config_path)
{
    if (!utils::fs::CheckFileExtension(config_path, ".conf") ||
        config_path[config_path.size() - 6] == '/') {
        throw std::invalid_argument("Invalid config file suffix.");
    }

    std::ifstream config_file(config_path.c_str());
    if (!config_file.is_open()) {
        throw std::invalid_argument("Couldn't open config file.");
    }

    ParsedConfig parser(config_file, "", "");
    InheritedSettings inherited_settings = {};
    return Config::Build(parser, inherited_settings);
}

void Config::Print() const
{
    LOG(DEBUG) << "--Configuration: --";
    LOG(DEBUG) << "Multiplex type: " << mx_type_;
    LOG(DEBUG) << "Error log path: " << error_log_path_;
    LOG(DEBUG) << "Error log level: " << error_log_level_;
    http_config_.Print();
}

static MxType ParseMxType(const std::string& val)
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

static MxType BuildMxType(const std::vector<std::string>& vals)
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

bool Config::IsKeyAllowed(const std::string& key)
{
    return key == "use" || key == "error_log";
}

bool Config::IsNestingAllowed(const ParsedConfig& f)
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

Config Config::Build(const ParsedConfig& f, const InheritedSettings& inherited_settings)
{
    for (std::map<std::string, std::string>::const_iterator it = f.settings().begin();
         it != f.settings().end(); ++it) {
        if (!IsKeyAllowed(it->first)) {
            throw std::runtime_error("Invalid configuration file: invalid key: " + it->first);
        }
    }
    if (!IsNestingAllowed(f)) {
        throw std::runtime_error("Invalid configuration file: invalid nesting.");
    }
    if (!inherited_settings.root.empty() || !inherited_settings.def_file.empty() ||
        !inherited_settings.dir_listing.empty()) {
        throw std::runtime_error("Invalid configuration file: invalid settings for main block.");
    }
    MxType mx_type = BuildMxType(f.FindSetting("use"));
    std::pair<std::string, Severity> error_log = BuildErrorLog(f.FindSetting("error_log"));
    HttpConfig http_conf = HttpConfig::Build(f.FindNesting("http")[0], inherited_settings);
    return Config(mx_type, error_log, http_conf);
}

}  // namespace config
