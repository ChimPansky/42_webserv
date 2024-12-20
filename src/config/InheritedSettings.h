#ifndef WS_CONFIG_INHERITED_SETTINGS_H
#define WS_CONFIG_INHERITED_SETTINGS_H

#include <string>
#include <vector>

namespace config {

struct InheritedSettings {
    std::string alias;
    std::vector<std::string> def_files;
    std::string dir_listing;
    std::string client_max_body_size;

  public:
    InheritedSettings()
        : alias(std::string()), def_files(std::vector<std::string>()), dir_listing(std::string())
    {}

    static const std::string& BuildAliasDir(const std::vector<std::string>& vals,
                                            const std::string& inherited_alias);
    static std::vector<std::string> BuildDefaultFile(
        const std::vector<std::string>& vals, const std::vector<std::string>& inherited_def_files);
    static const std::string& ParseDirListing(const std::string& vals);
    static const std::string& BuildDirListing(const std::vector<std::string>& vals,
                                              const std::string& inherited_dir_listing);
    static const std::string& BuildClientMaxBodySize(
        const std::vector<std::string>& vals, const std::string& inherited_client_max_body_size);
    static const std::string& ParseClientMaxBodySize(const std::string& val,
                                                     const std::string& unit);
};

}  // namespace config

#endif  // WS_CONFIG_INHERITED_SETTINGS_H
