#ifndef WS_CONFIG_PARSED_CONFIG_H
#define WS_CONFIG_PARSED_CONFIG_H

#include <utils.h>

#include <fstream>
#include <map>
#include <string>
#include <vector>

namespace config {

class ParsedConfig {
  public:
    ParsedConfig(std::ifstream& ifs, const std::string& nesting_lvl = "",
                 const std::string& nesting_lvl_descrt = "");

    typedef std::pair<std::string, std::string> Setting;
    const std::string& nesting_lvl() const;        // f. e. "http", "server", "location"
    const std::string& nesting_lvl_descr() const;  // for location lvl only, f. e. "/cgi-bin/"
    const std::multimap<std::string, std::string>& settings() const;
    const std::vector<ParsedConfig>& nested_configs() const;

    std::vector<std::string> FindSetting(const std::string& key) const;

  private:
    std::string nesting_lvl_;
    std::string nesting_lvl_descr_;
    std::multimap<std::string, std::string> settings_;
    std::vector<ParsedConfig> nested_configs_;
};

std::pair<std::string, std::string> MakePair(const std::string& line);

}  // namespace config

#endif  // WS_CONFIG_PARSED_CONFIG_H
