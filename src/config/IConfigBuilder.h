#ifndef WS_CONFIG_I_CONFIG_BUILDER_H
#define WS_CONFIG_I_CONFIG_BUILDER_H

#include <unistd.h>

#include <string>

#include "InheritedSettings.h"
#include "ParsedConfig.h"

namespace config {

template <class ConfigType>
class IConfigBuilder {
  protected:
    virtual bool IsKeyAllowed(const std::string& key) const = 0;
    virtual bool AreNestingsValid(const ParsedConfig& f) const = 0;

  public:
    virtual ConfigType Build(const ParsedConfig& f,
                             const InheritedSettings& inherited_settings) const = 0;
};

}  // namespace config

#endif  // WS_CONFIG_CONFIG_BUILDER_H
