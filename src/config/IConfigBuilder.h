#ifndef WS_CONFIG_I_CONFIG_BUILDER_H
#define WS_CONFIG_I_CONFIG_BUILDER_H

#include <unistd.h>

#include <string>

#include "ParsedConfig.h"

namespace config {

struct InheritedSettings {
    std::string root;
    std::vector<std::string> def_file;
    std::string dir_listing;

  public:
    InheritedSettings()
        : root(std::string()), def_file(std::vector<std::string>()), dir_listing(std::string())
    {}

    static const std::string& BuildRootDir(const std::vector<std::string>& vals,
                                           const std::string& inherited_root);
    static std::vector<std::string> BuildDefaultFile(
        const std::vector<std::string>& vals, const std::vector<std::string>& inherited_def_file);
    static const std::string& ParseDirListing(const std::string& vals);
    static const std::string& BuildDirListing(const std::vector<std::string>& vals,
                                              const std::string& inherited_dir_listing);
};

template <class ConfigType>
class IConfigBuilder {
  protected:
    virtual bool IsKeyAllowed(const std::string& key) const = 0;
    virtual bool CheckAllNestings(const ParsedConfig& f) const = 0;

  public:
    virtual ConfigType Build(const ParsedConfig& f,
                             const InheritedSettings& inherited_settings) const = 0;
};

}  // namespace config

#endif  // WS_CONFIG_CONFIG_BUILDER_H
