#include "LocationConfigBuilder.h"

#include <numeric_utils.h>
#include <str_utils.h>
#include <sys/types.h>

namespace config {

static std::pair<std::string, bool> ParseRoute(const std::vector<std::string>& vals)
{
    std::string route;
    bool priority = false;
    if (vals.size() > 2) {
        throw std::runtime_error("Invalid configuration file: invalid route: " + vals[0]);
    } else if (vals.size() == 1 && vals[0][0] == '/') {
        route = vals[0];
    } else if (vals[0] == "=" && vals[1][0] == '/') {
        priority = true;
        route = vals[1];
    } else {
        throw std::runtime_error("Invalid configuration file: invalid route: " + vals[0]);
    }
    return std::make_pair(route, priority);
}

static std::pair<std::string, bool> BuildRoute(const std::string& vals)
{
    if (vals.empty()) {
        throw std::runtime_error("Invalid configuration file: no route specified.");
    }
    std::vector<std::string> route_elements = utils::SplitLine(vals);
    return ParseRoute(route_elements);
}

static std::vector<http::Method> ParseAllowedMethods(const std::vector<std::string>& vals)
{
    std::vector<http::Method> allowed_methods;

    for (size_t i = 0; i < vals.size(); i++) {
        if (vals[i] == "GET") {
            allowed_methods.push_back(http::HTTP_GET);
        } else if (vals[i] == "POST") {
            allowed_methods.push_back(http::HTTP_POST);
        } else if (vals[i] == "DELETE") {
            allowed_methods.push_back(http::HTTP_DELETE);
        } else {
            throw std::runtime_error("Invalid configuration file: invalid method: " + vals[i]);
        }
    }
    return allowed_methods;
}

static std::vector<http::Method> BuildAllowedMethods(const std::vector<std::string>& vals)
{
    std::vector<std::string> allowed_methods;
    if (vals.empty()) {
        return LocationConfig::kDefaultAllowedMethods();
    }
    for (size_t i = 0; i < vals.size(); i++) {
        if (vals[i].empty()) {
            throw std::runtime_error("Invalid configuration file: no allowed methods specified.");
        }
        std::vector<std::string> val_elements = utils::SplitLine(vals[0]);
        allowed_methods.insert(allowed_methods.end(), val_elements.begin(), val_elements.end());
    }
    return ParseAllowedMethods(allowed_methods);
}

static std::pair<int, std::string> ParseRedirect(const std::vector<std::string>& vals)
{
    if (vals.size() != 2) {
        throw std::runtime_error("Invalid configuration file: redirection status code is invalid.");
    }
    int code = utils::StrToNumeric<int>(vals[0]);
    return std::make_pair(code, vals[1]);
}

static std::pair<int, std::string> BuildRedirect(const std::vector<std::string>& vals)
{
    if (vals.empty()) {
        return std::make_pair(LocationConfig::kDefaultRedirectCode(),
                              LocationConfig::kDefaultRedirectPath());
    } else if (vals.size() > 1) {
        throw std::runtime_error("Invalid configuration file: duplicated return value.");
    }
    std::vector<std::string> val_elements = utils::SplitLine(vals[0]);
    return ParseRedirect(val_elements);
}

static std::vector<std::string> BuildCgiPaths(const std::vector<std::string>& vals)
{
    if (vals.empty()) {
        return LocationConfig::kDefaultCgiPath();
    } else if (vals.size() > 1) {
        throw std::runtime_error("Invalid configuration file: duplicated cgi_path value.");
    } else if (vals[0][0] != '/') {
        throw std::runtime_error("Invalid configuration file: cgi_path isn't a directory.");
    }
    return vals;
}

static const std::vector<std::string>& ParseCgiExtensions(const std::vector<std::string>& vals)
{
    for (size_t i = 0; i < vals.size(); i++) {
        if (vals[i][0] != '.' || vals[i].length() < 2) {
            throw std::runtime_error("Invalid configuration file: invalid cgi_extension: " +
                                     vals[i]);
        }
    }
    return vals;
}

static std::vector<std::string> BuildCgiExtensions(const std::vector<std::string>& vals)
{
    std::vector<std::string> cgi_extensions;

    if (vals.empty()) {
        return LocationConfig::kDefaultCgiExtensions();
    }
    for (size_t i = 0; i < vals.size(); i++) {
        if (vals[i].empty()) {
            throw std::runtime_error("Invalid configuration file: no cgi extension specified.");
        }
        std::vector<std::string> val_elements = utils::SplitLine(vals[0]);
        cgi_extensions.insert(cgi_extensions.end(), val_elements.begin(), val_elements.end());
    }
    return ParseCgiExtensions(cgi_extensions);
}

static const std::string BuildAliasDir(const std::vector<std::string>& vals,
                                       const std::string& inherited_alias)
{
    if (vals.empty() && inherited_alias.empty()) {
        return LocationConfig::kDefaultAliasDir();
    }
    return InheritedSettings::BuildAliasDir(vals, inherited_alias);
}

static const std::string BuildUploadDir(const std::vector<std::string>& vals)
{
    if (vals.empty()) {
        return std::string();
    } else if (vals.size() > 1) {
        throw std::runtime_error("Invalid configuration file: duplicated upload_store value.");
    } else if (vals[0][0] != '/') {
        throw std::runtime_error("Invalid configuration file: upload_store isn't a directory.");
    }
    return vals[0];
}

static std::vector<std::string> BuildDefaultFile(
    const std::vector<std::string>& vals, const std::vector<std::string>& inherited_def_files)
{
    if (vals.empty() && inherited_def_files.empty()) {
        return LocationConfig::kDefaultIndexFile();
    }
    return InheritedSettings::BuildDefaultFile(vals, inherited_def_files);
}

static bool ParseDirListing(const std::string& vals)
{
    if (vals == "on") {
        return true;
    } else if (vals == "off") {
        return false;
    }
    throw std::runtime_error("Invalid configuration file: invalid autoindex value: " + vals);
}

static bool BuildDirListing(const std::vector<std::string>& vals,
                            const std::string& inherited_redirect)
{
    if (vals.empty() && inherited_redirect.empty()) {
        return LocationConfig::kDefaultDirListing();
    }
    return ParseDirListing(InheritedSettings::BuildDirListing(vals, inherited_redirect));
}

unsigned int ParseClientMaxBodySize(const std::string& val, const std::string& unit)
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

unsigned int BuildClientMaxBodySize(const std::vector<std::string>& vals,
                                    const std::string& inherited_client_max_body_size)
{
    if (vals.empty() && inherited_client_max_body_size.empty()) {
        return LocationConfig::kDefaultClientMaxBodySize();
    } else if (vals.size() > 1) {
        throw std::runtime_error("Invalid configuration file: duplicated client_max_body_size.");
    }
    std::vector<std::string> val_elements = utils::SplitLine(
        InheritedSettings::BuildClientMaxBodySize(vals, inherited_client_max_body_size));
    if (val_elements.size() == 1) {
        return ParseClientMaxBodySize(val_elements[0], "");
    } else if (val_elements.size() == 2) {
        return ParseClientMaxBodySize(val_elements[0], val_elements[1]);
    }
    throw std::runtime_error("Invalid configuration file: invalid client_max_body_size: " +
                             vals[0]);
}

bool LocationConfigBuilder::IsKeyAllowed(const std::string& key) const
{
    return key == "limit_except" || key == "return" || key == "cgi_path" ||
           key == "cgi_extension" || key == "alias" || key == "index" || key == "autoindex" ||
           key == "client_max_body_size" || key == "upload_store";
}

bool LocationConfigBuilder::AreNestingsValid(const ParsedConfig& f) const
{
    if (!f.nested_configs().empty()) {
        return false;
    }
    return true;
}

LocationConfig LocationConfigBuilder::Build(const ParsedConfig& f,
                                            const InheritedSettings& inherited_settings) const
{
    for (std::map<std::string, std::string>::const_iterator it = f.settings().begin();
         it != f.settings().end(); ++it) {
        if (!IsKeyAllowed(it->first)) {
            throw std::runtime_error("Invalid configuration file: invalid key: " + it->first);
        }
    }
    std::pair<std::string, bool> route = BuildRoute(f.nesting_lvl_descr());
    std::vector<http::Method> allowed_methods = BuildAllowedMethods(f.FindSetting("limit_except"));
    std::pair<int, std::string> redirect = BuildRedirect(f.FindSetting("return"));
    std::vector<std::string> cgi_paths = BuildCgiPaths(f.FindSetting("cgi_path"));
    std::vector<std::string> cgi_extensions = BuildCgiExtensions(f.FindSetting("cgi_extension"));
    std::string alias_dir = BuildAliasDir(f.FindSetting("alias"), inherited_settings.alias);
    std::vector<std::string> default_file =
        BuildDefaultFile(f.FindSetting("index"), inherited_settings.def_files);
    bool dir_listing = BuildDirListing(f.FindSetting("autoindex"), inherited_settings.dir_listing);
    unsigned int client_max_body_size = BuildClientMaxBodySize(
        f.FindSetting("client_max_body_size"), inherited_settings.client_max_body_size);
    std::string upload_dir = BuildUploadDir(f.FindSetting("upload_store"));

    if (!AreNestingsValid(f)) {
        throw std::runtime_error("Invalid configuration file: invalid nesting.");
    }
    return LocationConfig(route, allowed_methods, redirect, cgi_paths, cgi_extensions, alias_dir,
                          default_file, dir_listing, client_max_body_size, upload_dir);
}

}  // namespace config
