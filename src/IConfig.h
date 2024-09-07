#ifndef WS_ICONFIG_H
#define WS_ICONFIG_H

#include <map>
#include <string>

class   IConfig {

  public:
    virtual ~IConfig();
    typedef std::pair<std::string, std::string> Setting;
    typedef void (IConfig::*FunctionPointer)(const std::string&);
    virtual std::map<std::string, FunctionPointer> InitSettings() const = 0;
    typedef std::map<std::string, FunctionPointer>::const_iterator MethodsIt;
};

#endif  // WS_ICONFIG_H
