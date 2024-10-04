#include "IConfigBuilder.h"

namespace config {

const std::string& InheritedSettings::BuildRootDir(const std::vector<std::string>& vals,
                                                   const std::string& inherited_root)
{
    if (vals.empty()) {
        return inherited_root;
    } else if (vals.size() > 1) {
        throw std::runtime_error("Invalid configuration file: duplicated root value.");
    } else if (vals[0][0] != '/') {
        throw std::runtime_error("Invalid configuration file: root isn't a directory.");
    }
    return vals[0];
}

std::vector<std::string> InheritedSettings::BuildDefaultFile(
    const std::vector<std::string>& vals, const std::vector<std::string>& inherited_def_file)
{
    std::vector<std::string> default_files;

    if (vals.empty()) {
        return inherited_def_file;
    }
    for (size_t i = 0; i < vals.size(); i++) {
        if (vals[i].empty()) {
            throw std::runtime_error("Invalid configuration file: no index file specified.");
        }
        std::vector<std::string> val_elements = utils::fs::SplitLine(vals[0]);
        default_files.insert(default_files.end(), val_elements.begin(), val_elements.end());
    }
    return default_files;
}

const std::string& InheritedSettings::ParseDirListing(const std::string& vals)
{
    if (vals == "on" || vals == "off") {
        return vals;
    }
    throw std::runtime_error("Invalid configuration file: invalid autoindex value: " + vals);
}

const std::string& InheritedSettings::BuildDirListing(const std::vector<std::string>& vals,
                                                      const std::string& inherited_dir_listing)
{
    if (vals.empty()) {
        return inherited_dir_listing;
    } else if (vals.size() > 1) {
        throw std::runtime_error("Invalid configuration file: duplicated autoindex value.");
    }
    return ParseDirListing(vals[0]);
}

}  // namespace config
