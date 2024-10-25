#include "InheritedSettings.h"

#include <stdexcept>

#include <utils.h>

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

const std::string& InheritedSettings::ParseClientMaxBodySize(const std::string& val,
                                                             const std::string& unit)
{
    try {
        utils::StrToNumeric<int>(val);
    } catch (const std::invalid_argument& e) {
        throw std::runtime_error(
            "Invalid configuration file: invalid client_max_body_size value: " + val);
    }
    if (unit == "KB" || unit == "MB" || unit == "GB" || unit.empty()) {
        return val;
    }
    throw std::runtime_error("Invalid configuration file: invalid client_max_body_size unit: " +
                             val);
}

const std::string& InheritedSettings::BuildClientMaxBodySize(
    const std::vector<std::string>& vals, const std::string& inherited_client_max_body_size)
{
    if (vals.empty()) {
        return inherited_client_max_body_size;
    } else if (vals.size() > 1) {
        throw std::runtime_error("Invalid configuration file: duplicated client_max_body_size.");
    }
    std::vector<std::string> val_elements = utils::fs::SplitLine(vals[0]);
    if (val_elements.size() == 1) {
        ParseClientMaxBodySize(val_elements[0], "");
        return vals[0];
    } else if (val_elements.size() == 2) {
        ParseClientMaxBodySize(val_elements[0], val_elements[1]);
        return vals[0];
    }
    throw std::runtime_error("Invalid configuration file: invalid client_max_body_size: " +
                             vals[0]);
}

}  // namespace config
