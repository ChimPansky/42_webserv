#include "LocationConfig.h"

#include "utils/logger.h"

namespace config {

LocationConfig::LocationConfig(const std::pair<std::string, Priority>& route,
                               const std::vector<Method>& allowed_methods,
                               const std::pair<int, std::string>& redirect,
                               const std::vector<std::string>& cgi_paths,
                               const std::vector<std::string>& cgi_extensions,
                               const std::string& root_dir,
                               const std::vector<std::string>& default_file, bool dir_listing)
    : route_(route), allowed_methods_(allowed_methods), redirect_(InitRedirect(redirect)),
      is_cgi_(route.first == "/cgi-bin/"), cgi_paths_(cgi_paths), cgi_extensions_(cgi_extensions),
      root_dir_(root_dir), default_file_(default_file), dir_listing_(dir_listing)
{}

const std::pair<std::string, LocationConfig::Priority>& LocationConfig::route() const
{
    return route_;
}

const std::vector<LocationConfig::Method>& LocationConfig::allowed_methods() const
{
    return allowed_methods_;
}

const std::pair<int, std::string>& LocationConfig::redirect() const
{
    return redirect_;
}

bool LocationConfig::is_cgi() const
{
    return is_cgi_;
}

const std::vector<std::string>& LocationConfig::cgi_paths() const
{
    return cgi_paths_;
}

const std::vector<std::string>& LocationConfig::cgi_extensions() const
{
    return cgi_extensions_;
}

const std::string& LocationConfig::root_dir() const
{
    return root_dir_;
}

const std::vector<std::string>& LocationConfig::default_file() const
{
    return default_file_;
}

bool LocationConfig::dir_listing() const
{
    return dir_listing_;
}

std::pair<int, std::string> LocationConfig::InitRedirect(const std::pair<int, std::string>& value)
{
    if (value.first < 300 || value.first > 399) {
        throw std::runtime_error("Invalid configuration file: invalid redirect status code.");
    }
    return value;
}

void LocationConfig::Print() const
{
    LOG(DEBUG) << "\n";
    LOG(DEBUG) << "--Location configuration: --";
    LOG(DEBUG) << "Route: " << route_.first << " " << route_.second;
    LOG(DEBUG) << "Allowed methods: ";
    for (size_t i = 0; i < allowed_methods_.size(); i++) {
        LOG(DEBUG) << "  " << allowed_methods_[i];
    }
    LOG(DEBUG) << "Redirect: " << redirect_.first << " " << redirect_.second;
    LOG(DEBUG) << "CGI: " << (is_cgi_ ? "enabled" : "disabled");
    LOG(DEBUG) << "CGI paths: ";
    for (size_t i = 0; i < cgi_paths_.size(); i++) {
        LOG(DEBUG) << "  " << cgi_paths_[i];
    }
    LOG(DEBUG) << "CGI extensions: ";
    for (size_t i = 0; i < cgi_extensions_.size(); i++) {
        LOG(DEBUG) << "  " << cgi_extensions_[i];
    }
    LOG(DEBUG) << "Root directory: " << root_dir_;
    LOG(DEBUG) << "Default file: ";
    for (size_t i = 0; i < default_file_.size(); i++) {
        LOG(DEBUG) << "  " << default_file_[i];
    }
    LOG(DEBUG) << "Directory listing: " << dir_listing_;
}

std::pair<std::string, LocationConfig::Priority> ParseRoute(const std::vector<std::string>& vals)
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

std::pair<std::string, LocationConfig::Priority> BuildRoute(const std::string& vals)
{
    if (vals.empty()) {
        throw std::runtime_error("Invalid configuration file: no route specified.");
    }
    std::vector<std::string> route_elements = utils::fs::SplitLine(vals);
    return ParseRoute(route_elements);
}

std::vector<LocationConfig::Method> ParseAllowedMethods(const std::vector<std::string>& vals)
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

std::vector<LocationConfig::Method> BuildAllowedMethods(const std::vector<std::string>& vals)
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

std::pair<int, std::string> ParseRedirect(const std::vector<std::string>& vals)
{
    if (vals.size() != 2) {
        throw std::runtime_error("Invalid configuration file: redirection status code is invalid.");
    }
    int code = utils::StrToNumeric<int>(vals[0]);
    return std::make_pair(code, vals[1]);
}

std::pair<int, std::string> BuildRedirect(const std::vector<std::string>& vals)
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

std::vector<std::string> BuildCgiPaths(const std::vector<std::string>& vals)
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

const std::vector<std::string>& ParseCgiExtensions(const std::vector<std::string>& vals)
{
    for (size_t i = 0; i < vals.size(); i++) {
        if (vals[i] != ".py" && vals[i] != ".php") {
            throw std::runtime_error("Invalid configuration file: invalid cgi_extension: " +
                                     vals[i]);
        }
    }
    return vals;
}

std::vector<std::string> BuildCgiExtensions(const std::vector<std::string>& vals)
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

std::string BuildRootDir(const std::vector<std::string>& vals, const std::string& inherited_root)
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

std::vector<std::string> BuildDefaultFile(const std::vector<std::string>& vals,
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

bool ParseDirListing(const std::string& vals)
{
    if (vals == "on") {
        return true;
    } else if (vals == "off") {
        return false;
    }
    throw std::runtime_error("Invalid configuration file: invalid autoindex value: " + vals);
}

bool BuildDirListing(const std::vector<std::string>& vals, const std::string& inherited_redirect)
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

bool LocationConfig::IsKeyAllowed(const std::string& key)
{
    return key == "limit_except" || key == "return" || key == "cgi_path" ||
           key == "cgi_extension" || key == "root" || key == "index" || key == "autoindex";
}
bool LocationConfig::IsNestingAllowed(const ParsedConfig& f)
{
    if (!f.nested_configs().empty()) {
        return false;
    }
    return true;
}

LocationConfig LocationConfig::Build(const ParsedConfig& f,
                                     const InheritedSettings& inherited_settings)
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
