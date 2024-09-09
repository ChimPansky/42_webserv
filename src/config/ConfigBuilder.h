#ifndef WS_CONFIG_CONFIGBUILDER_H
#define WS_CONFIG_CONFIGBUILDER_H

#include <map>
#include <string>
#include "Config.h"
#include "ConfigParser.h"
#include "../utils.h/logger.h"

typedef c_api::EventManager::MultiplexType MxType;

template <class ConfigType>
class   ConfigBuilder {
    static ConfigType Build(const std::vector<ConfigParser>& f);
    static bool IsKeyAllowed(const std::string& key);
    static bool IsNestedAllowed(const std::string& nested_name);
};

template <>
class ConfigBuilder<Config> {

    static const MxType kDefaultMxType = MxType::MT_SELECT;
    static const std::string kDefaultErrorLogPath = "/logs/error.log";
    static const std::string kDefaultErrorLogLevel = "info";

    static MxType BuildMxType(const std::vector<const std::string&>& vals) {
        if (vals.empty()) {
            return kDefaultMxType;
        }
        return ParseMxType(vals[0]);
    }

    static MxType ParseMxType(const std::string& val) {
        if (val == "epoll") {
            return MxType::MT_EPOLL;
        } else if (val == "select") {
            return MxType::MT_SELECT;
        } else if (val == "poll") {
            return MxType::MT_POLL;
        } else {
            throw std::runtime_error("Invalid configuration file: invalid mx_type: " + val);
        }
    }

    static const std::string& BuildErrorLog(const std::vector<const std::string&>& vals, bool is_level) {
        if (val.empty()) {
            if (is_level) {
                return kDefaultErrorLogLevel;
            }
            return kDefaultErrorLogPath;
        }
        std::vector<std::string>    val_elements = SplitLine(vals[0]);
        if (val_elements.size() != 2) {
            throw std::runtime_error("Invalid configuration file: invalid error_log: " + vals[0]);
        } else if (is_level) {
            return ParseErrorLogLevel(val_elements[1]);
        }
        return ParseErrorLogPath(val_elements[1]);
    }

    static const std::string& ParseErrorLogPath(const std::string& val) {
        
        if (access(val.c_str(), F_OK) == -1) {
            throw std::runtime_error("Invalid configuration file: error_log file doesn't exist: " + val);
        }
        return val;
    }

    static Severity   ParseErrorLogLevel(const std::string& val) {
    
        if (val == "debug") {
            return Severity::DEBUG;
        } else if (val == "info") {
            return Severity::INFO;
        } else if (val == "warning") {
            return Severity::WARNING;
        } else if (val == "error") {
            return Severity::ERROR;
        } else if (val == "fatal") {
            return Severity::FATAL;
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

    static Config Build(const ConfigParser& f) {

        MxType mx_type = BuildMxType(f.FindSettings("use"));
        std::string error_log_path = BuildErrorLog(f.FindSetting("error_log"), false);
        Severity error_log_level = BuildErrorLog(f.FindSetting("error_log"), true);
        HttpConfig http_conf = ConfigBuilder<HttpConfig>::Build(f.FindNesting("http")[0]);

        for (std::vector<ConfigParser::Setting>::const_iterator it = f.settings().begin(); it != f.settings().end(); ++it) {
            if (!IsKeyAllowed(it->first)) {
                throw std::runtime_error("Invalid configuration file: invalid key: " + it->first);
            }
        }
        return Config(mx_type, error_log_path, error_log_level, http_conf);
    }
};

#endif  // WS_CONFIG_CONFIGBUILDER_H

/*main {
    Parser parser(file_name, "", "");

    ConfigBuilder config_builder(parser);

    Config = config_builder.Build();

    Server(config);
}*/
