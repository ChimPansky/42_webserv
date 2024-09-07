#ifndef WS_CONFIG_H
#define WS_CONFIG_H

#include "HttpConfig.h"

#include <iostream>
#include "utils/shared_ptr.h"

class Config : public IConfig {

  private:
    Config();
    static const std::string kDefaultMxType;
    static const std::string kDefaultErrorLogPath;
    static const std::string kDefaultErrorLogLevel;
    virtual std::map<std::string, FunctionPointer> InitSettings() const;
    void InitMxType(const std::string& value);
    void InitErrorLog(const std::string& value);

  public:
    Config(std::vector<Setting> settings);
    const std::string& mx_type() const;
    const std::string& error_log_path() const;
    const std::string& error_log_level() const;

  private:
    std::string mx_type_;
    std::string error_log_path_;
    std::string error_log_level_;
    utils::shared_ptr<HttpConfig> http_config_;
};

#endif  // WS_CONFIG_H
