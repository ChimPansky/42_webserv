#ifndef WS_CONFIG_HTTP_CONFIG_BUILDER_H
#define WS_CONFIG_HTTP_CONFIG_BUILDER_H

#include <string>

#include "HttpConfig.h"
#include "IConfigBuilder.h"

namespace config {

class HttpConfigBuilder : public IConfigBuilder<HttpConfig> {
  private:
    bool IsKeyAllowed(const std::string& key) const;
    bool AreNestingsValid(const ParsedConfig& f) const;

  public:
    HttpConfig Build(const ParsedConfig& f,
                     const InheritedSettings& inherited_settings = InheritedSettings()) const;
};

}  // namespace config

#endif  // WS_CONFIG_HTTP_CONFIG_BUILDER_H
