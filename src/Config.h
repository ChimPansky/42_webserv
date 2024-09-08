#ifndef WS_CONFIG_H
#define WS_CONFIG_H

#include "HttpConfig.h"
#include "utils/shared_ptr.h"

class Config : public IConfig {

  private:
    static const std::string kDefaultMxType;
    static const std::string kDefaultErrorLogPath;
    static const std::string kDefaultErrorLogLevel;
    virtual bool  IsValid() const;
    void InitMxType(const std::string& value);
    void InitErrorLog(const std::string& value);

  public:
    Config(std::vector<Setting> settings, utils::shared_ptr<HttpConfig> http_config);
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
