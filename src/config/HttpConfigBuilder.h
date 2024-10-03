#ifndef WS_CONFIG_HTTP_CONFIG_BUILDER_H
#define WS_CONFIG_HTTP_CONFIG_BUILDER_H

#include <unistd.h>

#include <string>

#include "HttpConfig.h"
#include "IConfigBuilder.h"

namespace config {

class HttpConfigBuilder : public IConfigBuilder<HttpConfig> {
  private:
    bool IsKeyAllowed(const std::string& key) const;
    bool IsNestingAllowed(const ParsedConfig& f) const;

  public:
    HttpConfig Build(const ParsedConfig& f, const InheritedSettings& inherited_settings) const;
};

}  // namespace config

#endif  // WS_CONFIG_HTTP_CONFIG_BUILDER_H
