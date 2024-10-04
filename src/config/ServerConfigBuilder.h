#ifndef WS_CONFIG_SERVER_CONFIG_BUILDER_H
#define WS_CONFIG_SERVER_CONFIG_BUILDER_H

#include <unistd.h>

#include <string>

#include "IConfigBuilder.h"
#include "ServerConfig.h"

namespace config {

class ServerConfigBuilder : public IConfigBuilder<ServerConfig> {
  private:
    bool IsKeyAllowed(const std::string& key) const;
    bool IsNestingAllowed(const ParsedConfig& f) const;

  public:
    ServerConfig Build(const ParsedConfig& f, const InheritedSettings& inherited_settings) const;
};

}  // namespace config

#endif  // WS_CONFIG_SERVER_CONFIG_BUILDER_H
