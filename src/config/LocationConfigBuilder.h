#ifndef WS_CONFIG_LOCATION_CONFIG_BUILDER_H
#define WS_CONFIG_LOCATION_CONFIG_BUILDER_H

#include <unistd.h>

#include <string>

#include "IConfigBuilder.h"
#include "LocationConfig.h"

namespace config {

class LocationConfigBuilder : public IConfigBuilder<LocationConfig> {
  private:
    bool IsKeyAllowed(const std::string& key) const;
    bool AreNestingsValid(const ParsedConfig& f) const;

  public:
    LocationConfig Build(const ParsedConfig& f, const InheritedSettings& inherited_settings) const;
};

}  // namespace config

#endif  // WS_CONFIG_LOCATION_CONFIG_BUILDER_H
