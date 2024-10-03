#include "ServerConfig.h"

#include "LocationConfig.h"
#include "c_api/utils.h"

namespace config {

ServerConfig::ServerConfig(const std::pair<std::string, Severity>& access_log,
                           const std::string& error_log_path,
                           const std::vector<std::pair<in_addr_t, in_port_t> >& listeners,
                           const std::vector<std::string>& server_names,
                           const std::vector<LocationConfig>& locations)
    : access_log_path_(access_log.first), access_log_level_(access_log.second),
      error_log_path_(error_log_path), listeners_(listeners), server_names_(server_names),
      locations_(locations)
{}

const std::string& ServerConfig::access_log_path() const
{
    return access_log_path_;
}

Severity ServerConfig::access_log_level() const
{
    return access_log_level_;
}

const std::string& ServerConfig::error_log_path() const
{
    return error_log_path_;
}

const std::vector<std::pair<in_addr_t, in_port_t> >& ServerConfig::listeners() const
{
    return listeners_;
}

const std::vector<std::string>& ServerConfig::server_names() const
{
    return server_names_;
}

const std::vector<LocationConfig>& ServerConfig::locations() const
{
    return locations_;
}

void ServerConfig::Print() const
{
    LOG(DEBUG) << "\n";
    LOG(DEBUG) << "--Server configuration: --";
    LOG(DEBUG) << "Access log path: " << access_log_path_;
    LOG(DEBUG) << "Access log level: " << access_log_level_;
    LOG(DEBUG) << "Error log path: " << error_log_path_;
    LOG(DEBUG) << "Listeners: ";
    for (size_t i = 0; i < listeners_.size(); i++) {
        LOG(DEBUG) << "  " << c_api::IPv4ToString(listeners_[i].first) << ":"
                   << listeners_[i].second;
    }
    LOG(DEBUG) << "Server names:";
    for (size_t i = 0; i < server_names_.size(); i++) {
        LOG(DEBUG) << "  " << server_names_[i];
    }
    for (size_t i = 0; i < locations_.size(); i++) {
        locations_[i].Print();
    }
}

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
    }
    std::vector<std::string> val_elements = utils::fs::SplitLine(vals[0]);
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

    if (val.find(':') == std::string::npos) {
        if (val.find('.') != std::string::npos) {
            addr = c_api::IPv4FromString(val);
            port = ServerConfig::kDefaultPort();
        } else {
            addr = c_api::IPv4FromString(ServerConfig::kDefaultIPAddress());
            port = utils::StrToNumeric<in_port_t>(val);
        }
    } else {
        size_t colon_pos = val.find(':');
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

static bool IsValidComponent(const std::string& label)
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

    size_t start = 0;
    size_t dotPos = 0;
    while ((dotPos = val.find('.', start)) != std::string::npos) {
        if (!(start == 0 && val.substr(start, dotPos - start) == "*")) {
            if (!IsValidComponent(val.substr(start, dotPos - start)))
                throw std::runtime_error("Invalid server name: invalid domain label");
        }
        start = dotPos + 1;
    }
    if (val.substr(start) != "*" && !IsValidComponent(val.substr(start)))
        throw std::runtime_error("Invalid server name: invalid domain label");

    return val;
}

static std::vector<std::string> ParseServerNames(const std::vector<std::string>& vals)
{
    std::vector<std::string> server_names;

    for (size_t i = 0; i < vals.size(); i++) {
        std::vector<std::string> val_elements = utils::fs::SplitLine(vals[i]);
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

static const std::string BuildRootDir(const std::vector<std::string>& vals,
                                      const std::string& inherited_root)
{
    if (vals.empty() && inherited_root.empty()) {
        return std::string();
    } else if (vals.empty()) {
        return inherited_root;
    } else if (vals.size() > 1) {
        throw std::runtime_error("Invalid configuration file: duplicated root value.");
    } else if (vals[0][0] != '/') {
        throw std::runtime_error("Invalid configuration file: root isn't a directory.");
    }
    return vals[0];
}

static std::vector<std::string> BuildDefaultFile(const std::vector<std::string>& vals,
                                                 const std::vector<std::string>& inherited_def_file)
{
    std::vector<std::string> default_files;
    if (vals.empty() && inherited_def_file.empty()) {
        return std::vector<std::string>();
    } else if (vals.empty()) {
        return inherited_def_file;
    }
    for (size_t i = 0; i < vals.size(); i++) {
        if (vals[i].empty()) {
            throw std::runtime_error("Invalid configuration file: no index file specified.");
        }
        std::vector<std::string> val_elements = utils::fs::SplitLine(vals[0]);
        default_files.insert(default_files.end(), val_elements.begin(), val_elements.end());
    }
    return default_files;
}

static std::string ParseDirListing(const std::string& vals)
{
    if (vals == "on" || vals == "off") {
        return vals;
    }
    throw std::runtime_error("Invalid configuration file: invalid autoindex value: " + vals);
}

static const std::string BuildDirListing(const std::vector<std::string>& vals,
                                         const std::string& inherited_redirect)
{
    if (vals.empty() && inherited_redirect.empty()) {
        return std::string();
    } else if (vals.empty()) {
        return inherited_redirect;
    } else if (vals.size() > 1) {
        throw std::runtime_error("Invalid configuration file: duplicated autoindex value.");
    }
    return ParseDirListing(vals[0]);
}

static std::vector<LocationConfig> BuildLocationConfigs(
    const std::vector<ParsedConfig>& nested_configs, const InheritedSettings& inherited_settings)
{
    std::vector<LocationConfig> server_configs;
    for (size_t i = 0; i < nested_configs.size(); i++) {
        server_configs.push_back(LocationConfig::Build(nested_configs[i], inherited_settings));
    }
    return server_configs;
}

bool ServerConfig::IsKeyAllowed(const std::string& key)
{
    return key == "listen" || key == "server_name" || key == "access_log" || key == "error_log" ||
           key == "root" || key == "index" || key == "autoindex";
}

bool ServerConfig::IsNestingAllowed(const ParsedConfig& f)
{
    if (f.nested_configs().empty()) {
        return false;
    }
    for (std::vector<ParsedConfig>::const_iterator it = f.nested_configs().begin();
         it != f.nested_configs().end(); ++it) {
        if (it->nesting_lvl_descr().empty() || it->nesting_lvl() != "location") {
            return false;
        }
    }
    return true;
}

ServerConfig ServerConfig::Build(const ParsedConfig& f, const InheritedSettings& inherited_settings)
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
    std::pair<std::string, Severity> access_log = BuildAccessLog(f.FindSetting("access_log"));
    std::string error_log_path = BuildErrorLogPath(f.FindSetting("error_log"));
    std::vector<std::pair<in_addr_t, in_port_t> > listeners =
        BuildListeners(f.FindSetting("listen"));
    std::vector<std::string> server_names = BuildServerNames(f.FindSetting("server_name"));

    InheritedSettings server_inherited_settings = {};
    server_inherited_settings.root = BuildRootDir(f.FindSetting("root"), inherited_settings.root);
    server_inherited_settings.def_file =
        BuildDefaultFile(f.FindSetting("index"), inherited_settings.def_file);
    server_inherited_settings.dir_listing =
        BuildDirListing(f.FindSetting("autoindex"), inherited_settings.dir_listing);
    std::vector<LocationConfig> location_configs =
        BuildLocationConfigs(f.FindNesting("location"), server_inherited_settings);

    return ServerConfig(access_log, error_log_path, listeners, server_names, location_configs);
}

}  // namespace config
