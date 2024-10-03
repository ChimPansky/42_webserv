#include "HttpConfig.h"

namespace config {

HttpConfig::HttpConfig(size_t keepalive_timeout, size_t client_max_body_size,
                       const std::map<int, std::string>& error_pages,
                       const std::vector<ServerConfig>& server_configs)
    : keepalive_timeout_(keepalive_timeout), client_max_body_size_(client_max_body_size),
      error_pages_(InitErrorPages(error_pages)), server_configs_(server_configs)
{}

size_t HttpConfig::keepalive_timeout() const
{
    return keepalive_timeout_;
}

size_t HttpConfig::client_max_body_size() const
{
    return client_max_body_size_;
}

const std::map<int, std::string>& HttpConfig::error_pages() const
{
    return error_pages_;
}

const std::vector<ServerConfig>& HttpConfig::server_configs() const
{
    return server_configs_;
}

const std::map<int, std::string>& HttpConfig::InitErrorPages(
    const std::map<int, std::string>& value)
{
    typedef std::map<int, std::string>::const_iterator ErrorPagesIt;
    for (ErrorPagesIt it = value.begin(); it != value.end(); ++it) {
        if (it->first < 400 || it->first > 599) {
            throw std::runtime_error("Invalid configuration file: invalid error_page status code.");
        }
    }
    return value;
}

void config::HttpConfig::Print() const
{
    LOG(DEBUG) << "\n";
    LOG(DEBUG) << "--HTTP configuration: --";
    LOG(DEBUG) << "Keepalive timeout: " << keepalive_timeout_;
    LOG(DEBUG) << "Client max body size: " << client_max_body_size_;
    LOG(DEBUG) << "Error pages:";
    for (std::map<int, std::string>::const_iterator it = error_pages_.begin();
         it != error_pages_.end(); ++it) {
        LOG(DEBUG) << "  " << it->first << " -> " << it->second;
    }
    for (size_t i = 0; i < server_configs_.size(); i++) {
        server_configs_[i].Print();
    }
}

static size_t ParseKeepAliveTimeout(const std::string& val)
{
    return utils::StrToNumeric<size_t>(val);
}

static size_t BuildKeepAliveTimeout(const std::vector<std::string>& vals)
{
    if (vals.empty()) {
        return HttpConfig::kDefaultKeepaliveTimeout();
    } else if (vals.size() > 1) {
        throw std::runtime_error("Invalid configuration file: duplicated keepalive_timeout.");
    }
    return ParseKeepAliveTimeout(vals[0]);
}

static size_t ParseClientMaxBodySize(const std::string& val, const std::string& unit)
{
    size_t nb = utils::StrToNumeric<int>(val);
    if (unit == "KB") {
        return nb << 10;
    } else if (unit == "MB") {
        return nb << 20;
    } else if (unit == "GB") {
        return nb << 30;
    } else if (unit.empty()) {
        return nb;
    }
    throw std::runtime_error("Invalid configuration file: invalid client_max_body_size unit: " +
                             val);
}

static size_t BuildClientMaxBodySize(const std::vector<std::string>& vals)
{
    if (vals.empty()) {
        return HttpConfig::kDefaultClientMaxBodySize();
    } else if (vals.size() > 1) {
        throw std::runtime_error("Invalid configuration file: duplicated client_max_body_size.");
    }
    std::vector<std::string> val_elements = utils::fs::SplitLine(vals[0]);
    if (val_elements.size() == 1) {
        return ParseClientMaxBodySize(val_elements[0], "");
    } else if (val_elements.size() == 2) {
        return ParseClientMaxBodySize(val_elements[0], val_elements[1]);
    }
    throw std::runtime_error("Invalid configuration file: invalid client_max_body_size: " +
                             vals[0]);
}

static std::map<int, std::string> ParseErrorPages(const std::vector<std::string>& vals)
{
    std::map<int, std::string> error_pages;

    for (size_t i = 0; i < vals.size(); i++) {
        std::vector<std::string> val_elements = utils::fs::SplitLine(vals[i]);
        if (val_elements.size() < 2) {
            throw std::runtime_error("Invalid configuration file: invalid error_page: " + vals[i]);
        } /* else if (access(val_elements[val_elements.size() - 1].c_str(), F_OK | R_OK) == -1)
        { throw std::runtime_error("Invalid configuration file: error page doesn't exist: " +
                                        val_elements[val_elements.size() - 1]); // temprorary
        comment for valgrind CI test
        } */
        for (size_t j = 0; j < val_elements.size() - 1; j++) {
            if (j != val_elements.size() - 1) {
                error_pages[utils::StrToNumeric<int>(val_elements[j])] =
                    val_elements[val_elements.size() - 1];
            }
        }
    }
    return error_pages;
}

static std::map<int, std::string> BuildErrorPages(const std::vector<std::string>& vals)
{
    if (vals.empty()) {
        return std::map<int, std::string>();
    }
    return ParseErrorPages(vals);
}

static std::string BuildRootDir(const std::vector<std::string>& vals)
{
    if (vals.empty()) {
        return std::string();
    } else if (vals.size() > 1) {
        throw std::runtime_error("Invalid configuration file: duplicated root value.");
    } else if (vals[0][0] != '/') {
        throw std::runtime_error("Invalid configuration file: root isn't a directory.");
    }
    return vals[0];
}

static std::vector<std::string> BuildDefaultFile(const std::vector<std::string>& vals)
{
    std::vector<std::string> default_files;
    if (vals.empty()) {
        return std::vector<std::string>();
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

static const std::string& ParseDirListing(const std::string& vals)
{
    if (vals == "on" || vals == "off") {
        return vals;
    }
    throw std::runtime_error("Invalid configuration file: invalid autoindex value: " + vals);
}

static std::string BuildDirListing(const std::vector<std::string>& vals)
{
    if (vals.empty()) {
        return std::string();
    } else if (vals.size() > 1) {
        throw std::runtime_error("Invalid configuration file: duplicated autoindex value.");
    }
    return ParseDirListing(vals[0]);
}

static std::vector<ServerConfig> BuildServerConfigs(const std::vector<ParsedConfig>& nested_configs,
                                                    const InheritedSettings& inherited_settings)
{
    std::vector<ServerConfig> server_configs;
    for (size_t i = 0; i < nested_configs.size(); i++) {
        server_configs.push_back(ServerConfig::Build(nested_configs[i], inherited_settings));
    }
    return server_configs;
}

bool HttpConfig::IsKeyAllowed(const std::string& key)
{
    return key == "keepalive_timeout" || key == "client_max_body_size" || key == "error_page" ||
           key == "root" || key == "index" || key == "autoindex";
}

bool HttpConfig::IsNestingAllowed(const ParsedConfig& f)
{
    if (f.nested_configs().empty()) {
        return false;
    }
    for (std::vector<ParsedConfig>::const_iterator it = f.nested_configs().begin();
         it != f.nested_configs().end(); ++it) {
        if (!it->nesting_lvl_descr().empty() || it->nesting_lvl() != "server") {
            return false;
        }
    }
    return true;
}

HttpConfig HttpConfig::Build(const ParsedConfig& f, const InheritedSettings& inherited_settings)
{
    if (!inherited_settings.root.empty() || !inherited_settings.def_file.empty() ||
        !inherited_settings.dir_listing.empty()) {
        throw std::runtime_error("Invalid configuration file: invalid settings for http block.");
    }
    for (std::map<std::string, std::string>::const_iterator it = f.settings().begin();
         it != f.settings().end(); ++it) {
        if (!IsKeyAllowed(it->first)) {
            throw std::runtime_error("Invalid configuration file: invalid key: " + it->first);
        }
    }
    if (!IsNestingAllowed(f)) {
        throw std::runtime_error("Invalid configuration file: invalid nesting.");
    }
    size_t keepalive_timeout = BuildKeepAliveTimeout(f.FindSetting("keepalive_timeout"));
    size_t client_max_body_size = BuildClientMaxBodySize(f.FindSetting("client_max_body_size"));
    std::map<int, std::string> error_pages = BuildErrorPages(f.FindSetting("error_page"));

    InheritedSettings http_inherited_settings = {};
    http_inherited_settings.root = BuildRootDir(f.FindSetting("root"));
    http_inherited_settings.def_file = BuildDefaultFile(f.FindSetting("index"));
    http_inherited_settings.dir_listing = BuildDirListing(f.FindSetting("autoindex"));
    std::vector<ServerConfig> server_configs =
        BuildServerConfigs(f.FindNesting("server"), http_inherited_settings);
    return HttpConfig(keepalive_timeout, client_max_body_size, error_pages, server_configs);
}

}  // namespace config
