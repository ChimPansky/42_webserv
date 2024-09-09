#ifndef WS_CONFIG_CONFIGBUILDER_H
#define WS_CONFIG_CONFIGBUILDER_H

#include "Config.h"
#include "ConfigParser.h"

#include <unistd.h>

typedef c_api::EventManager::MultiplexType MxType;

template <class ConfigType>
class   ConfigBuilder {

    private:
        static bool IsKeyAllowed(const std::string& key);
        static bool IsNestedAllowed(const std::string& nested_name);
    public:
        static ConfigType Build(const ConfigParser& f);
};


template <>
class ConfigBuilder<Config> {

    private:
        static const MxType kDefaultMxType = c_api::EventManager::MT_SELECT;
        static const std::string kDefaultErrorLogPath;
        static const Severity kDefaultErrorLogLevel = INFO;

    static MxType BuildMxType(const std::vector<std::string>& vals) {
        if (vals.empty()) {
            return kDefaultMxType;
        }
        return ParseMxType(vals[0]);
    }

    static MxType ParseMxType(const std::string& val) {
        if (val == "epoll") {
            return c_api::EventManager::MT_EPOLL;
        } else if (val == "select") {
            return c_api::EventManager::MT_SELECT;
        } else if (val == "poll") {
            return c_api::EventManager::MT_POLL;
        }
        throw std::runtime_error("Invalid configuration file: invalid mx_type: " + val);
    }

    static const std::string& BuildErrorLogPath(const std::vector<std::string>& vals) {

        if (vals.empty()) {
            return kDefaultErrorLogPath;
        }
        std::vector<std::string>    val_elements = SplitLine(vals[0]); // config utils
        if (val_elements.size() != 2) {
            throw std::runtime_error("Invalid configuration file: invalid error_log: " + vals[0]);
        }
        return ParseErrorLogPath(val_elements[0]);
    }

    static const std::string& ParseErrorLogPath(const std::string& val) {
        
        if (access(val.c_str(), F_OK) == -1) {
            throw std::runtime_error("Invalid configuration file: error_log file doesn't exist: " + val);
        }
        return val;
    }

    static Severity BuildErrorLogLevel(const std::vector<std::string>& vals) {

        if (vals.empty()) {
            return kDefaultErrorLogLevel;
        }
        std::vector<std::string>    val_elements = SplitLine(vals[0]); // config utils
        if (val_elements.size() != 2) {
            throw std::runtime_error("Invalid configuration file: invalid error_log: " + vals[0]);
        }
        return ParseErrorLogLevel(val_elements[1]);
    }

    static Severity   ParseErrorLogLevel(const std::string& val) {
    
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
        } else {
            throw std::runtime_error("Invalid configuration file: invalid error_log level: " + val);
        }
    }

    static std::vector<std::string> SplitLine(const std::string& line) {
    
        std::vector<std::string>    elements;
        size_t  start = 0;
        size_t  end = 0;
        while (end != std::string::npos) {
            end = line.find(' ', start);
            elements.push_back(line.substr(start, end - start));
            start = end + 1;
        }
        return elements;
    }

    static bool IsKeyAllowed(const std::string& key) {
        return key == "use" || key == "error_log" || key == "http";
    }

    public:
        static Config Build(const ConfigParser& f) {
            MxType mx_type = BuildMxType(f.FindSetting("use"));
            std::string error_log_path = BuildErrorLogPath(f.FindSetting("error_log"));
            Severity error_log_level = BuildErrorLogLevel(f.FindSetting("error_log"));
            HttpConfig http_conf = ConfigBuilder<HttpConfig>::Build(f.FindNesting("http", 0));

            for (std::map<std::string, std::string>::const_iterator it = f.settings().begin(); it != f.settings().end(); ++it) {
                if (!IsKeyAllowed(it->first)) {
                    throw std::runtime_error("Invalid configuration file: invalid key: " + it->first);
                }
            }
            return Config(mx_type, error_log_path, error_log_level, http_conf);
        }
};

const std::string ConfigBuilder<Config>::kDefaultErrorLogPath = "/log/error.log";

#endif  // WS_CONFIG_CONFIGBUILDER_H
