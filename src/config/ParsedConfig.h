#ifndef WS_CONFIG_PARSED_CONFIG_H
#define WS_CONFIG_PARSED_CONFIG_H

#include <config/utils.h>

#include <fstream>
#include <map>
#include <string>
#include <vector>

namespace config {

class ParsedConfig {
  public:
    ParsedConfig(std::ifstream& ifs, const std::string& lvl, const std::string& lvl_descrt = "");

    typedef std::pair<std::string, std::string> Setting;
    const std::string& lvl() const;
    const std::string& lvl_descr() const;
    const std::multimap<std::string, std::string>& settings() const;
    const std::vector<ParsedConfig>& nested_configs() const;

    std::vector<std::string> FindSetting(const std::string& key) const;
    const std::vector<ParsedConfig>& FindNesting(const std::string& key) const;

  private:
    std::string lvl_;
    std::string lvl_descr_;
    std::multimap<std::string, std::string> settings_;
    std::vector<ParsedConfig> nested_configs_;
};

}  // namespace config

#endif  // WS_CONFIG_PARSED_CONFIG_H
