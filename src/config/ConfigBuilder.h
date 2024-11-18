#ifndef WS_CONFIG_CONFIG_BUILDER_H
#define WS_CONFIG_CONFIG_BUILDER_H

#include <unistd.h>

#include <string>

#include "Config.h"
#include "IConfigBuilder.h"

namespace config {

class ConfigBuilder : public IConfigBuilder<Config> {
  private:
    bool IsKeyAllowed(const std::string& key) const;
    bool AreNestingsValid(const ParsedConfig& f) const;

  public:
    typedef c_api::MultiplexType MxType;

    Config Build(const ParsedConfig& f,
                 const InheritedSettings& inherited_settings = InheritedSettings()) const;
    static const Config GetConfigFromConfFile(const std::string& config_path);
};

}  // namespace config

#endif  // WS_CONFIG_CONFIG_BUILDER_H
