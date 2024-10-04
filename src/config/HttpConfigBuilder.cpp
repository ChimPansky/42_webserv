#include "HttpConfigBuilder.h"

#include "ServerConfigBuilder.h"

namespace config {

static unsigned int ParseKeepAliveTimeout(const std::string& val)
{
    return utils::StrToNumeric<unsigned int>(val);
}

static unsigned int BuildKeepAliveTimeout(const std::vector<std::string>& vals)
{
    if (vals.empty()) {
        return HttpConfig::kDefaultKeepaliveTimeout();
    } else if (vals.size() > 1) {
        throw std::runtime_error("Invalid configuration file: duplicated keepalive_timeout.");
    }
    return ParseKeepAliveTimeout(vals[0]);
}

static unsigned int ParseClientMaxBodySize(const std::string& val, const std::string& unit)
{
    unsigned int nb = utils::StrToNumeric<int>(val);
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

static unsigned int BuildClientMaxBodySize(const std::vector<std::string>& vals)
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

static std::vector<ServerConfig> BuildServerConfigs(const std::vector<ParsedConfig>& nested_configs,
                                                    const InheritedSettings& inherited_settings)
{
    std::vector<ServerConfig> server_configs;
    for (size_t i = 0; i < nested_configs.size(); i++) {
        ServerConfigBuilder server_config_builder;
        server_configs.push_back(
            server_config_builder.Build(nested_configs[i], inherited_settings));
    }
    return server_configs;
}

bool HttpConfigBuilder::IsKeyAllowed(const std::string& key) const
{
    return key == "keepalive_timeout" || key == "client_max_body_size" || key == "error_page" ||
           key == "root" || key == "index" || key == "autoindex";
}

bool HttpConfigBuilder::CheckAllNestings(const ParsedConfig& f) const
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

HttpConfig HttpConfigBuilder::Build(const ParsedConfig& f,
                                    const InheritedSettings& inherited_settings) const
{
    for (std::map<std::string, std::string>::const_iterator it = f.settings().begin();
         it != f.settings().end(); ++it) {
        if (!IsKeyAllowed(it->first)) {
            throw std::runtime_error("Invalid configuration file: invalid key: " + it->first);
        }
    }
    unsigned int keepalive_timeout = BuildKeepAliveTimeout(f.FindSetting("keepalive_timeout"));
    unsigned int client_max_body_size =
        BuildClientMaxBodySize(f.FindSetting("client_max_body_size"));
    std::map<int, std::string> error_pages = BuildErrorPages(f.FindSetting("error_page"));

    InheritedSettings http_inherited_settings = inherited_settings;
    http_inherited_settings.root =
        InheritedSettings::BuildRootDir(f.FindSetting("root"), inherited_settings.root);
    http_inherited_settings.def_file =
        InheritedSettings::BuildDefaultFile(f.FindSetting("index"), inherited_settings.def_file);
    http_inherited_settings.dir_listing = InheritedSettings::BuildDirListing(
        f.FindSetting("autoindex"), inherited_settings.dir_listing);

    if (!CheckAllNestings(f)) {
        throw std::runtime_error("Invalid configuration file: invalid nesting.");
    }
    std::vector<ServerConfig> server_configs =
        BuildServerConfigs(f.nested_configs(), http_inherited_settings);
    return HttpConfig(keepalive_timeout, client_max_body_size, error_pages, server_configs);
}
}  // namespace config
