#include "LocationConfigBuilder.h"

#include <sys/types.h>

namespace config {

static std::pair<std::string, LocationConfig::Priority> ParseRoute(
    const std::vector<std::string>& vals)
{
    std::string route;
    LocationConfig::Priority priority = LocationConfig::P1;
    if (vals.size() > 2) {
        throw std::runtime_error("Invalid configuration file: invalid route: " + vals[0]);
    } else if (vals.size() == 1 && vals[0][0] == '/') {
        route = vals[0];
    } else if (vals[0] == "=" && vals[1][0] == '/') {
        priority = LocationConfig::P0;
        route = vals[1];
    } else {
        throw std::runtime_error("Invalid configuration file: invalid route: " + vals[0]);
    }
    return std::make_pair(route, priority);
}

static std::pair<std::string, LocationConfig::Priority> BuildRoute(const std::string& vals)
{
    if (vals.empty()) {
        throw std::runtime_error("Invalid configuration file: no route specified.");
    }
    std::vector<std::string> route_elements = utils::fs::SplitLine(vals);
    return ParseRoute(route_elements);
}

static std::vector<LocationConfig::Method> ParseAllowedMethods(const std::vector<std::string>& vals)
{
    std::vector<LocationConfig::Method> allowed_methods;

    for (size_t i = 0; i < vals.size(); i++) {
        if (vals[i] == "GET") {
            allowed_methods.push_back(LocationConfig::GET);
        } else if (vals[i] == "POST") {
            allowed_methods.push_back(LocationConfig::POST);
        } else if (vals[i] == "DELETE") {
            allowed_methods.push_back(LocationConfig::DELETE);
        } else {
            throw std::runtime_error("Invalid configuration file: invalid method: " + vals[i]);
        }
    }
    return allowed_methods;
}

static std::vector<LocationConfig::Method> BuildAllowedMethods(const std::vector<std::string>& vals)
{
    std::vector<std::string> allowed_methods;
    if (vals.empty()) {
        return LocationConfig::kDefaultAllowedMethods();
    }
    for (size_t i = 0; i < vals.size(); i++) {
        if (vals[i].empty()) {
            throw std::runtime_error("Invalid configuration file: no allowed methods specified.");
        }
        std::vector<std::string> val_elements = utils::fs::SplitLine(vals[0]);
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
    std::vector<std::string> val_elements = utils::fs::SplitLine(vals[0]);
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
        if (vals[i] != ".py" && vals[i] != ".php") {
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
        std::vector<std::string> val_elements = utils::fs::SplitLine(vals[0]);
        cgi_extensions.insert(cgi_extensions.end(), val_elements.begin(), val_elements.end());
    }
    return ParseCgiExtensions(cgi_extensions);
}

static const std::string BuildRootDir(const std::vector<std::string>& vals,
                                      const std::string& inherited_root)
{
    if (vals.empty() && inherited_root.empty()) {
        return LocationConfig::kDefaultRootDir();
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
        return LocationConfig::kDefaultIndexFile();
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
    } else if (vals.size() > 1) {
        throw std::runtime_error("Invalid configuration file: duplicated autoindex value.");
    } else if (vals.empty()) {
        return ParseDirListing(inherited_redirect);
    }
    return ParseDirListing(vals[0]);
}

bool LocationConfigBuilder::IsKeyAllowed(const std::string& key) const
{
    return key == "limit_except" || key == "return" || key == "cgi_path" ||
           key == "cgi_extension" || key == "root" || key == "index" || key == "autoindex";
}
bool LocationConfigBuilder::IsNestingAllowed(const ParsedConfig& f) const
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
    if (!IsNestingAllowed(f)) {
        throw std::runtime_error("Invalid configuration file: invalid nesting.");
    }
    std::pair<std::string, LocationConfig::Priority> route = BuildRoute(f.nesting_lvl_descr());
    std::vector<LocationConfig::Method> allowed_methods =
        BuildAllowedMethods(f.FindSetting("limit_except"));
    std::pair<int, std::string> redirect = BuildRedirect(f.FindSetting("return"));
    std::vector<std::string> cgi_paths = BuildCgiPaths(f.FindSetting("cgi_path"));
    std::vector<std::string> cgi_extensions = BuildCgiExtensions(f.FindSetting("cgi_extension"));
    std::string root_dir = BuildRootDir(f.FindSetting("root"), inherited_settings.root);
    std::vector<std::string> default_file =
        BuildDefaultFile(f.FindSetting("index"), inherited_settings.def_file);
    bool dir_listing = BuildDirListing(f.FindSetting("autoindex"), inherited_settings.dir_listing);

    return LocationConfig(route, allowed_methods, redirect, cgi_paths, cgi_extensions, root_dir,
                          default_file, dir_listing);
}
}  // namespace config
