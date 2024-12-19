#include "ServerConfigBuilder.h"

#include <c_api_utils.h>
#include <numeric_utils.h>
#include <str_utils.h>

#include <stdexcept>

#include "LocationConfig.h"
#include "LocationConfigBuilder.h"
#include "ServerConfig.h"

namespace config {

static Severity ParseAccessLogLevel(const std::string& val)
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
    } else if (val == "") {
        return ServerConfig::kDefaultAccessLogLevel();
    }
    throw std::runtime_error("Invalid configuration file: invalid access_log level: " + val);
}

static std::pair<std::string, Severity> BuildAccessLog(const std::vector<std::string>& vals)
{
    if (vals.empty()) {
        return std::make_pair(ServerConfig::kDefaultAccessLogPath(),
                              ServerConfig::kDefaultAccessLogLevel());
    } else if (vals.size() > 1) {
        throw std::runtime_error("Invalid configuration file: duplicated access_log value.");
    }
    std::vector<std::string> val_elements = utils::SplitLine(vals[0]);
    if (val_elements.size() == 1) {
        return std::make_pair(val_elements[0], ServerConfig::kDefaultAccessLogLevel());
    } else if (val_elements.size() == 2) {
        return std::make_pair(val_elements[0], ParseAccessLogLevel(val_elements[1]));
    }
    throw std::runtime_error("Invalid configuration file: invalid access_log: " + vals[0]);
}

static std::string BuildErrorLogPath(const std::vector<std::string>& vals)
{
    if (vals.empty()) {
        return ServerConfig::kDefaultErrorLogPath();
    } else if (vals.size() > 1) {
        throw std::runtime_error("Invalid configuration file: duplicated error_log value.");
    }
    return vals[0];
}

static std::pair<in_addr_t, in_port_t> ParseListener(const std::string& val)
{
    in_addr_t addr;
    in_port_t port;

    size_t colon_pos = val.find(':');
    if (colon_pos == std::string::npos) {
        try {
            addr = c_api::IPv4FromString(val);
            port = ServerConfig::kDefaultPort();
        } catch (std::runtime_error& e) {
            addr = ServerConfig::kDefaultIPAddress();
            port = utils::StrToNumeric<in_port_t>(val);  // possible rethrow this line
        }
    } else {
        addr = c_api::IPv4FromString(val.substr(0, colon_pos));
        port = utils::StrToNumeric<in_port_t>(val.substr(colon_pos + 1));
    }
    return std::make_pair(addr, port);
}

static std::vector<std::pair<in_addr_t, in_port_t> > BuildListeners(
    const std::vector<std::string>& vals)
{
    if (vals.empty()) {
        throw std::runtime_error("Invalid configuration file: listen setting isn't specified.");
    }
    std::vector<std::pair<in_addr_t, in_port_t> > listeners;
    for (size_t i = 0; i < vals.size(); i++) {
        listeners.push_back(ParseListener(vals[i]));
    }
    return listeners;
}

static bool CheckComponentSeparatedByDots(const std::string& label)
{
    if (label.empty() || label[0] == '-' || label[label.size() - 1] == '-') {
        return false;
    }
    for (size_t i = 0; i < label.size(); ++i) {
        if (!std::isalnum(label[i]) && label[i] != '-') {
            return false;
        }
    }
    return true;
}

static const std::string& ParseServerName(const std::string& val)
{
    for (size_t i = 0; i < val.size(); i++) {
        if (isalnum(val[i]) || val[i] == '-' || val[i] == '.' || val[i] == '*') {
            if (val[i] == '*' && i != 0 && i != val.size() - 1) {
                throw std::runtime_error(
                    "Invalid server name: Wildcard '*' must be at the start or end");
            }
        } else {
            throw std::runtime_error("Invalid server name: invalid characters in server name");
        }
    }

    if (val[0] == '*' && val[val.size() - 1] == '*') {
        throw std::runtime_error("Invalid server name: invalid wildcard in server name");
    }

    size_t start = 0;
    size_t dotPos = 0;
    while ((dotPos = val.find('.', start)) != std::string::npos) {
        if (!(start == 0 && val.substr(start, dotPos - start) == "*")) {
            if (!CheckComponentSeparatedByDots(val.substr(start, dotPos - start)))
                throw std::runtime_error("Invalid server name: invalid domain label");
        }
        start = dotPos + 1;
    }
    if (val.substr(start) != "*" && !CheckComponentSeparatedByDots(val.substr(start)))
        throw std::runtime_error("Invalid server name: invalid domain label");

    return val;
}

static std::vector<std::string> ParseServerNames(const std::vector<std::string>& vals)
{
    std::vector<std::string> server_names;

    for (size_t i = 0; i < vals.size(); i++) {
        std::vector<std::string> val_elements = utils::SplitLine(vals[i]);
        if (!val_elements.empty()) {
            for (size_t j = 0; j < val_elements.size(); j++) {
                if (val_elements[j] == "\"\"") {
                    j++;
                } else {
                    server_names.push_back(ParseServerName(val_elements[j]));
                }
            }
        }
    }
    return server_names;
}

static std::vector<std::string> BuildServerNames(const std::vector<std::string>& vals)
{
    if (vals.empty()) {
        return std::vector<std::string>();
    }
    return ParseServerNames(vals);
}

static std::vector<LocationConfig> BuildLocationConfigs(
    const std::vector<ParsedConfig>& nested_configs, const InheritedSettings& inherited_settings)
{
    std::vector<LocationConfig> server_configs;
    for (size_t i = 0; i < nested_configs.size(); i++) {
        LocationConfigBuilder location_config_builder;
        server_configs.push_back(
            location_config_builder.Build(nested_configs[i], inherited_settings));
    }
    return server_configs;
}

bool ServerConfigBuilder::IsKeyAllowed(const std::string& key) const
{
    return key == "listen" || key == "server_name" || key == "access_log" || key == "error_log" ||
           key == "root" || key == "index" || key == "autoindex" || "client_max_body_size";
}

bool ServerConfigBuilder::AreNestingsValid(const ParsedConfig& f) const
{
    for (std::vector<ParsedConfig>::const_iterator it = f.nested_configs().begin();
         it != f.nested_configs().end(); ++it) {
        if (it->nesting_lvl_descr().empty() || it->nesting_lvl() != "location") {
            return false;
        }
    }
    return true;
}

ServerConfig ServerConfigBuilder::Build(const ParsedConfig& f,
                                        const InheritedSettings& inherited_settings) const
{
    for (std::map<std::string, std::string>::const_iterator it = f.settings().begin();
         it != f.settings().end(); ++it) {
        if (!IsKeyAllowed(it->first)) {
            throw std::runtime_error("Invalid configuration file: invalid key: " + it->first);
        }
    }
    std::pair<std::string, Severity> access_log = BuildAccessLog(f.FindSetting("access_log"));
    std::string error_log_path = BuildErrorLogPath(f.FindSetting("error_log"));
    std::vector<std::pair<in_addr_t, in_port_t> > listeners =
        BuildListeners(f.FindSetting("listen"));
    std::vector<std::string> server_names = BuildServerNames(f.FindSetting("server_name"));

    InheritedSettings server_inherited_settings = InheritedSettings();
    server_inherited_settings.root =
        InheritedSettings::BuildRootDir(f.FindSetting("root"), inherited_settings.root);
    server_inherited_settings.def_files =
        InheritedSettings::BuildDefaultFile(f.FindSetting("index"), inherited_settings.def_files);
    server_inherited_settings.dir_listing = InheritedSettings::BuildDirListing(
        f.FindSetting("autoindex"), inherited_settings.dir_listing);
    server_inherited_settings.client_max_body_size = InheritedSettings::BuildClientMaxBodySize(
        f.FindSetting("client_max_body_size"), inherited_settings.client_max_body_size);

    if (!AreNestingsValid(f)) {
        throw std::runtime_error("Invalid configuration file: invalid nesting.");
    }
    std::vector<LocationConfig> location_configs =
        BuildLocationConfigs(f.nested_configs(), server_inherited_settings);

    return ServerConfig(access_log, error_log_path, listeners, server_names, location_configs);
}
}  // namespace config
