#ifndef WS_CONFIG_CONFIGBUILDER_H
#define WS_CONFIG_CONFIGBUILDER_H

#include <unistd.h>

#include <cstddef>
#include <string>
#include <utility>

#include "Config.h"
#include "ConfigParser.h"
#include "c_api/utils.h"
#include "config/utils.h"

namespace config {

typedef c_api::MultiplexType MxType;

template <class ConfigType>
class ConfigBuilder {
  private:
    static bool IsKeyAllowed(const std::string& key);
    static bool IsNestingAllowed(const std::string& nested_name);

  public:
    static ConfigType Build(const ConfigParser& f, const std::string& inherited_root,
                            const std::string& inherited_index,
                            const std::string& inherited_autoindex);
};

template <>
class ConfigBuilder<LocationConfig> {
  private:
    static std::string BuildRoute(const std::string& vals)
    {
        if (vals.empty()) {
            throw std::runtime_error("Invalid configuration file: no route specified.");
        }
        std::vector<std::string> route_elements = config::SplitLine(vals);
        return ParseRoute(route_elements);
    }

    static std::string ParseRoute(const std::vector<std::string>& vals)
    {
        if (vals.size() > 2) {
            throw std::runtime_error("Invalid configuration file: invalid route: " + vals[0]);
        }
        if (vals.size() == 1) {
            return vals[0];
        }
        if (vals[0] == "=" || vals[0] == "^~") {
            // set priorities
        } else {
            throw std::runtime_error("Invalid configuration file: invalid route syntax: " +
                                     vals[0]);
        }
        return vals[1];
    }

    static std::vector<std::string> BuildAllowedMethods(const std::vector<std::string>& vals)
    {
        if (vals.empty()) {
            std::vector<std::string> allowed_methods;
            allowed_methods.push_back("GET");
            allowed_methods.push_back("POST");
            return allowed_methods;  // if setting is empty - use default allowed_methods
        }
        std::vector<std::string> val_elements = config::SplitLine(vals[0]);
        return ParseAllowedMethods(val_elements);
    }

    static const std::vector<std::string>& ParseAllowedMethods(const std::vector<std::string>& vals)
    {
        for (size_t i = 0; i < vals.size(); i++) {
            if (vals[i] != "GET" && vals[i] != "POST" && vals[i] != "DELETE") {
                throw std::runtime_error("Invalid configuration file: invalid method: " + vals[i]);
            }
        }
        return vals;
    }

    static std::pair<int, std::string> BuildRedirect(const std::vector<std::string>& vals)
    {
        if (vals.empty()) {
            return std::make_pair(LocationConfig::kDefaultRedirectCode,
                                  LocationConfig::kDefaultRedirectPath);
        }
        std::vector<std::string> val_elements = config::SplitLine(vals[0]);
        return ParseRedirect(val_elements);
    }

    static std::pair<int, std::string> ParseRedirect(const std::vector<std::string>& vals)
    {
        if (vals.size() != 2) {
            throw std::runtime_error("Invalid configuration file: invalid return: " + vals[0]);
        }
        int code = config::StrToInt(vals[0]);
        // if (access(vals[1].c_str(), F_OK | R_OK | W_OK) == -1) {
        //     throw std::runtime_error("Invalid configuration file: index file doesn't exist: " +
        //     vals[1]);
        // }
        return std::make_pair(code, vals[1]);
    }

    static std::vector<std::string> BuildCgiPaths(const std::vector<std::string>& vals)
    {
        if (vals.empty()) {
            return std::vector<std::string>();
        }
        return ParseCgiPaths(vals);
    }

    static const std::vector<std::string>& ParseCgiPaths(const std::vector<std::string>& vals)
    {
        for (size_t i = 0; i < vals.size(); i++) {
            if (!IsDirectory(vals[i])) {
                throw std::runtime_error("Invalid configuration file: cgi_path doesn't exist: " +
                                         vals[i]);
            }
        }
        return vals;
    }

    static std::vector<std::string> BuildCgiExtensions(const std::vector<std::string>& vals)
    {
        if (vals.empty()) {
            return std::vector<std::string>();
        }
        std::vector<std::string> val_elements = config::SplitLine(vals[0]);
        return ParseCgiExtensions(val_elements);
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

    static const std::string BuildRootDir(const std::vector<std::string>& vals,
                                          const std::string& inherited_root)
    {
        if (vals.empty() && inherited_root.empty()) {
            return LocationConfig::kDefaultRootDir;
        } else if (vals.empty()) {
            return inherited_root;
        }
        return ParseRootDir(vals[0]);
    }

    static const std::string& ParseRootDir(const std::string& val)
    {
        if (!IsDirectory(val)) {
            throw std::runtime_error("Invalid configuration file: root directory doesn't exist: " +
                                     val);
        }
        return val;
    }

    static const std::string BuildDefaultFile(const std::vector<std::string>& vals,
                                              const std::string& inherited_index)
    {
        if (vals.empty() && inherited_index.empty()) {
            return LocationConfig::kDefaultIndexFile;
        } else if (vals.empty()) {
            return inherited_index;
        }
        return ParseDefaultFile(vals[0]);
    }

    static const std::string& ParseDefaultFile(const std::string& vals)
    {
        // if (access(vals.c_str(), F_OK | R_OK | W_OK) == -1) {
        //     throw std::runtime_error("Invalid configuration file: index file doesn't exist: " +
        //     vals);
        // }
        return vals;
    }

    static const std::string BuildDirListing(const std::vector<std::string>& vals,
                                             const std::string& inherited_autoindex)
    {
        if (vals.empty() && inherited_autoindex.empty()) {
            return LocationConfig::kDefaultDirListing;
        } else if (vals.empty()) {
            return inherited_autoindex;
        }
        return ParseDirListing(vals[0]);
    }

    static const std::string& ParseDirListing(const std::string& vals)
    {
        if (vals == "on" || vals == "off") {
            return vals;
        }
        throw std::runtime_error("Invalid configuration file: invalid autoindex value: " + vals);
    }

    static bool IsKeyAllowed(const std::string& key)
    {
        return key == "allow_methods" || key == "return" || key == "cgi_path" ||
               key == "cgi_extension" || key == "root" || key == "index" || key == "autoindex";
    }

  public:
    static LocationConfig Build(const ConfigParser& f, const std::string& inherited_root,
                                const std::string& inherited_index,
                                const std::string& inherited_autoindex)
    {
        std::string route = BuildRoute(f.lvl_descr());
        std::vector<std::string> allowed_methods =
            BuildAllowedMethods(f.FindSetting("allow_methods"));
        std::pair<int, std::string> redirect = BuildRedirect(f.FindSetting("return"));
        std::vector<std::string> cgi_paths = BuildCgiPaths(f.FindSetting("cgi_path"));
        std::vector<std::string> cgi_extensions =
            BuildCgiExtensions(f.FindSetting("cgi_extension"));
        std::string root_dir = BuildRootDir(f.FindSetting("root"), inherited_root);
        std::string default_file = BuildDefaultFile(f.FindSetting("index"), inherited_index);
        std::string dir_listing = BuildDirListing(f.FindSetting("autoindex"), inherited_autoindex);

        for (std::map<std::string, std::string>::const_iterator it = f.settings().begin();
             it != f.settings().end(); ++it) {
            if (!IsKeyAllowed(it->first)) {
                throw std::runtime_error("Invalid configuration file: invalid key: " + it->first);
            }
        }

        return LocationConfig(route, allowed_methods, redirect, cgi_paths, cgi_extensions, root_dir,
                              default_file, dir_listing);
    }
};

template <>
class ConfigBuilder<ServerConfig> {
  private:
    static const std::string& ParseAccessLogPath(const std::string& vals)
    {
        // if (access(vals.c_str(), F_OK | R_OK | W_OK) == -1) {
        //     throw std::runtime_error("Invalid configuration file: access log doesn't exist: " +
        //     vals);
        // }
        return vals;
    }

    static std::string BuildAccessLogPath(const std::vector<std::string>& vals)
    {
        if (vals.empty()) {
            return ServerConfig::kDefaultAccessLogPath;
        }
        std::vector<std::string> val_elements = config::SplitLine(vals[0]);
        if (val_elements.size() < 2) {
            throw std::runtime_error("Invalid configuration file: invalid access_log: " + vals[0]);
        }
        return ParseAccessLogPath(val_elements[0]);
    }

    static Severity BuildAccessLogLevel(const std::vector<std::string>& vals)
    {
        if (vals.empty()) {
            return ServerConfig::kDefaultAccessLogLevel;
        }
        std::vector<std::string> val_elements = config::SplitLine(vals[0]);
        if (val_elements.size() != 2) {
            throw std::runtime_error("Invalid configuration file: invalid access_log: " + vals[1]);
        }
        return ParseAccessLogLevel(val_elements[1]);
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
            return ServerConfig::kDefaultAccessLogLevel;
        }
        throw std::runtime_error("Invalid configuration file: invalid access_log level: " + val);
    }

    static std::string BuildErrorLogPath(const std::vector<std::string>& vals)
    {
        if (vals.empty()) {
            return std::string();
        }
        return ParseErrorLogPath(vals[0]);
    }

    static std::string ParseErrorLogPath(const std::string& val)
    {
        if (access(val.c_str(), F_OK | R_OK | W_OK) == -1) {
            throw std::runtime_error("Invalid configuration file: error_log file doesn't exist: " +
                                     val);
        }
        return val;
    }

    static std::vector<std::pair<in_addr_t, in_port_t> > BuildListeners(
        const std::vector<std::string>& vals)
    {
        if (vals.empty()) {
            throw std::runtime_error("Invalid configuration file: no port specified.");
        }
        std::vector<std::pair<in_addr_t, in_port_t> > listeners;
        for (size_t i = 0; i < vals.size(); i++) {
            listeners.push_back(ParseListener(vals[i]));
        }
        return listeners;
    }

    static std::pair<in_addr_t, in_port_t> ParseListener(const std::string& val)
    {
        in_addr_t addr;
        in_port_t port;

        if (val.find(':') == std::string::npos) {
            addr = c_api::IPv4FromString("localhost");
            port = config::StrToInt(val);
        } else {
            size_t colon_pos = val.find(':');
            addr = c_api::IPv4FromString(val.substr(0, colon_pos));
            port = config::StrToInt(val.substr(colon_pos + 1));
        }

        // if (!(port > 0 && port <= 65535)) { // checking for valid port should be in a
        // ServerConfig method
        //     throw std::runtime_error("Invalid configuration file: invalid port: " + val);
        // }
        return std::make_pair(addr, port);
    }

    static std::vector<std::string> BuildServerNames(const std::vector<std::string>& vals)
    {
        if (vals.empty()) {
            return std::vector<std::string>();  // can it be empty?
        }
        return ParseServerNames(vals[0]);
    }

    static std::vector<std::string> ParseServerNames(const std::string& vals)
    {
        std::vector<std::string> server_names = config::SplitLine(vals);
        for (size_t i = 0; i < server_names.size(); i++) {
            if (server_names[i].empty()) {
                throw std::runtime_error("Invalid configuration file: invalid server_name: " +
                                         vals);
            }
        }  // check for possible server names
        return server_names;
    }

    static const std::string BuildRootDir(const std::vector<std::string>& vals,
                                          const std::string& inherited_root)
    {
        if (vals.empty() && inherited_root.empty()) {
            return std::string();
        } else if (vals.empty()) {
            return inherited_root;
        }
        return ParseRootDir(vals[0]);
    }

    static const std::string& ParseRootDir(const std::string& val)
    {
        if (!IsDirectory(val)) {
            throw std::runtime_error("Invalid configuration file: root directory doesn't exist: " +
                                     val);
        }
        return val;
    }

    static const std::string BuildDefaultFile(const std::vector<std::string>& vals,
                                              const std::string& inherited_index)
    {
        if (vals.empty() && inherited_index.empty()) {
            return std::string();
        } else if (vals.empty()) {
            return inherited_index;
        }
        return ParseDefaultFile(vals[0]);
    }

    static const std::string& ParseDefaultFile(const std::string& val)
    {
        // if (access(val.c_str(), F_OK | R_OK | W_OK) == -1) {
        //     throw std::runtime_error("Invalid configuration file: index file doesn't exist: " +
        //     val);
        // }
        return val;
    }

    static const std::string BuildDirListing(const std::vector<std::string>& vals,
                                             const std::string& inherited_autoindex)
    {
        if (vals.empty() && inherited_autoindex.empty()) {
            return std::string();
        } else if (vals.empty()) {
            return inherited_autoindex;
        }
        return ParseDirListing(vals[0]);
    }

    static std::string ParseDirListing(const std::string& vals)
    {
        if (vals == "on" || vals == "off") {
            return vals;
        }
        throw std::runtime_error("Invalid configuration file: invalid autoindex value: " + vals);
    }

    static std::vector<LocationConfig> BuildLocationConfigs(
        const std::vector<ConfigParser>& nested_configs, const std::string& inherited_root,
        const std::string& inherited_index, const std::string& inherited_autoindex)
    {
        std::vector<LocationConfig> server_configs;
        for (size_t i = 0; i < nested_configs.size(); i++) {
            server_configs.push_back(ConfigBuilder<LocationConfig>::Build(
                nested_configs[i], inherited_root, inherited_index, inherited_autoindex));
        }
        return server_configs;
    }

    static bool IsKeyAllowed(const std::string& key)
    {
        return key == "listen" || key == "server_name" || key == "access_log" ||
               key == "error_log" || key == "root" || key == "index" || key == "autoindex" ||
               key == "location";
    }

  public:
    static ServerConfig Build(const ConfigParser& f, const std::string& inherited_root,
                              const std::string& inherited_index,
                              const std::string& inherited_autoindex)
    {
        std::string access_log_path = BuildAccessLogPath(f.FindSetting("access_log"));
        Severity access_log_level = BuildAccessLogLevel(f.FindSetting("access_log"));
        std::string error_log_path = BuildErrorLogPath(f.FindSetting("error_log"));
        std::vector<std::pair<in_addr_t, in_port_t> > listeners =
            BuildListeners(f.FindSetting("listen"));
        std::vector<std::string> server_names = BuildServerNames(f.FindSetting("server_name"));
        std::string root_dir = BuildRootDir(f.FindSetting("root"), inherited_root);
        std::string default_file = BuildDefaultFile(f.FindSetting("index"), inherited_index);
        std::string dir_listing = BuildDirListing(f.FindSetting("autoindex"), inherited_autoindex);
        std::vector<LocationConfig> location_configs = BuildLocationConfigs(
            f.FindNesting("location"), root_dir, default_file, dir_listing);  // map or vector

        for (std::map<std::string, std::string>::const_iterator it = f.settings().begin();
             it != f.settings().end(); ++it) {
            if (!IsKeyAllowed(it->first)) {
                throw std::runtime_error("Invalid configuration file: invalid key: " + it->first);
            }
        }
        return ServerConfig(access_log_path, access_log_level, error_log_path, listeners,
                            server_names, location_configs);
    }
};

template <>
class ConfigBuilder<HttpConfig> {
  private:
    static bool IsKeyAllowed(const std::string& key)
    {
        return key == "keepalive_timeout" || key == "client_max_body_size" || key == "error_page" ||
               key == "root" || key == "index" || key == "autoindex";
    }

    static int BuildKeepAliveTimeout(const std::vector<std::string>& vals)
    {
        if (vals.empty()) {
            return HttpConfig::kDefaultKeepaliveTimeout;
        }
        return ParseKeepAliveTimeout(vals[0]);
    }

    static int ParseKeepAliveTimeout(const std::string& val)
    {
        return config::StrToInt(val);
    }

    static size_t BuildClientMaxBodySize(const std::vector<std::string>& vals)
    {
        if (vals.empty()) {
            return HttpConfig::kDefaultClientMaxBodySize;
        }
        std::vector<std::string> val_elements = config::SplitLine(vals[0]);
        if (val_elements.size() == 1) {
            return ParseClientMaxBodySize(val_elements[0], "");
        } else if (val_elements.size() == 2) {
            return ParseClientMaxBodySize(val_elements[0], val_elements[1]);
        }
        throw std::runtime_error("Invalid configuration file: invalid client_max_body_size: " +
                                 vals[0]);
    }

    static size_t ParseClientMaxBodySize(const std::string& val, const std::string& unit)
    {
        size_t nb = config::StrToInt(val);
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

    static std::map<int, std::string> BuildErrorPages(const std::vector<std::string>& vals)
    {
        if (vals.empty()) {
            return std::map<int, std::string>();
        }
        return ParseErrorPages(vals);
    }

    static std::map<int, std::string> ParseErrorPages(const std::vector<std::string>& vals)
    {
        std::map<int, std::string> error_pages;

        for (size_t i = 0; i < vals.size(); i++) {
            std::vector<std::string> val_elements = config::SplitLine(vals[i]);
            if (val_elements.size() < 2) {
                throw std::runtime_error("Invalid configuration file: invalid error_page: " +
                                         vals[i]);
            }
            for (size_t j = 0; j < val_elements.size() - 1; j++) {
                if (j != val_elements.size() - 1) {
                    error_pages[StrToInt(val_elements[j])] = val_elements[val_elements.size() - 1];
                }
            }
            // if (access(val_elements[val_elements.size() - 1].c_str(), F_OK | R_OK) == -1) {
            //     throw std::runtime_error("Invalid configuration file: error page doesn't exist: "
            //     // check if error page exists
            //     + val_elements[val_elements.size() - 1]);
            // }
        }
        return error_pages;
    }

    static std::string BuildRootDir(const std::vector<std::string>& vals)
    {
        if (vals.empty()) {
            return std::string();
        }
        return ParseRootDir(vals[0]);
    }

    static std::string ParseRootDir(const std::string& val)
    {
        if (!IsDirectory(val)) {
            throw std::runtime_error("Invalid configuration file: root directory doesn't exist: " +
                                     val);
        }
        return val;
    }

    static std::string BuildDefaultFile(const std::vector<std::string>& vals)
    {
        if (vals.empty()) {
            return std::string();
        }
        return ParseDefaultFile(vals[0]);
    }

    static std::string ParseDefaultFile(const std::string& val)
    {
        // if (access(val.c_str(), F_OK | R_OK | W_OK) == -1) {
        //     throw std::runtime_error("Invalid configuration file: index file doesn't exist: " +
        //     val);
        // }
        return val;
    }

    static std::string BuildDirListing(const std::vector<std::string>& vals)
    {
        if (vals.empty()) {
            return std::string();
        }
        return ParseDirListing(vals[0]);
    }

    static const std::string& ParseDirListing(const std::string& vals)
    {
        if (vals == "on" || vals == "off") {
            return vals;
        }
        throw std::runtime_error("Invalid configuration file: invalid autoindex value: " + vals);
    }

    static std::vector<ServerConfig> BuildServerConfigs(
        const std::vector<ConfigParser>& nested_configs, const std::string& inherited_root,
        const std::string& inherited_index, const std::string& inherited_autoindex)
    {
        std::vector<ServerConfig> server_configs;
        for (size_t i = 0; i < nested_configs.size(); i++) {
            server_configs.push_back(ConfigBuilder<ServerConfig>::Build(
                nested_configs[i], inherited_root, inherited_index, inherited_autoindex));
        }
        return server_configs;
    }

  public:
    static HttpConfig Build(const ConfigParser& f, const std::string& inherited_root,
                            const std::string& inherited_index,
                            const std::string& inherited_autoindex)
    {
        if (!inherited_root.empty() || !inherited_index.empty() || !inherited_autoindex.empty()) {
            throw std::runtime_error(
                "Invalid configuration file: invalid settings for http block.");
        }
        int keepalive_timeout = BuildKeepAliveTimeout(f.FindSetting("keepalive_timeout"));
        size_t client_max_body_size = BuildClientMaxBodySize(f.FindSetting("client_max_body_size"));
        std::map<int, std::string> error_pages = BuildErrorPages(f.FindSetting("error_page"));
        std::string root_dir = BuildRootDir(f.FindSetting("root"));
        std::string default_file = BuildDefaultFile(f.FindSetting("index"));
        std::string dir_listing = BuildDirListing(f.FindSetting("autoindex"));
        std::vector<ServerConfig> server_configs =
            BuildServerConfigs(f.FindNesting("server"), root_dir, default_file, dir_listing);
        for (std::map<std::string, std::string>::const_iterator it = f.settings().begin();
             it != f.settings().end(); ++it) {
            if (!IsKeyAllowed(it->first)) {
                throw std::runtime_error("Invalid configuration file: invalid key: " + it->first);
            }
        }
        return HttpConfig(keepalive_timeout, client_max_body_size, error_pages, server_configs);
    }
};

template <>
class ConfigBuilder<Config> {
  private:
    static MxType BuildMxType(const std::vector<std::string>& vals)
    {
        if (vals.empty()) {
            return Config::kDefaultMxType;
        }
        return ParseMxType(vals[0]);
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

    static std::pair<std::string, Severity> BuildErrorLog(const std::vector<std::string>& vals)
    {
        if (vals.empty()) {
            return std::make_pair(Config::kDefaultErrorLogPath, Config::kDefaultErrorLogLevel);
        }
        std::vector<std::string> val_elements = config::SplitLine(vals[0]);
        if (val_elements.size() == 1) {
            return std::make_pair(ParseErrorLogPath(val_elements[0]),
                                  Config::kDefaultErrorLogLevel);
        } else if (val_elements.size() == 2) {
            return std::make_pair(ParseErrorLogPath(val_elements[0]),
                                  ParseErrorLogLevel(val_elements[1]));
        }
        throw std::runtime_error("Invalid configuration file: invalid error_log setting: " +
                                 vals[0]);
    }

    static std::string ParseErrorLogPath(const std::string& val)
    {
        /* if (!config::ValidPath(val)) {
            throw std::runtime_error("Invalid configuration file: invalid error_log path: " + val);
        } */ // check for valid path
        return val;
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
            return Config::kDefaultErrorLogLevel;
        }
        throw std::runtime_error("Invalid configuration file: invalid error_log level: " + val);
    }

    static bool IsKeyAllowed(const std::string& key)
    {
        return key == "use" || key == "error_log";
    }

  public:
    static Config Build(const ConfigParser& f, const std::string& inherited_root,
                        const std::string& inherited_index, const std::string& inherited_autoindex)
    {
        if (!inherited_root.empty() || !inherited_index.empty() || !inherited_autoindex.empty()) {
            throw std::runtime_error(
                "Invalid configuration file: invalid settings for main block.");
        }
        MxType mx_type = BuildMxType(f.FindSetting("use"));
        std::pair<std::string, Severity> error_log = BuildErrorLog(f.FindSetting("error_log"));
        HttpConfig http_conf =
            ConfigBuilder<HttpConfig>::Build(f.FindNesting("http")[0], "", "", "");
        for (std::map<std::string, std::string>::const_iterator it = f.settings().begin();
             it != f.settings().end(); ++it) {
            if (!IsKeyAllowed(it->first)) {
                throw std::runtime_error("Invalid configuration file: invalid key: " + it->first);
            }
        }
        return Config(mx_type, error_log, http_conf);
    }
};

}  // namespace config

#endif  // WS_CONFIG_CONFIGBUILDER_H