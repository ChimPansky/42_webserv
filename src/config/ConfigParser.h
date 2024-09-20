#ifndef WS_CONFIG_CONFIG_PARSER_H
#define WS_CONFIG_CONFIG_PARSER_H

#include <fstream>
#include <string>
#include <map>
#include <vector>

namespace config {
class ConfigParser  {

  public:
    ConfigParser(std::ifstream& ifs, const std::string& lvl, const std::string& lvl_descrt = "");
    typedef std::pair<std::string, std::string>  Setting;
    const std::string& lvl() const;
    const std::string& lvl_descr() const;
    const std::multimap<std::string, std::string>& settings() const;
    const std::vector<ConfigParser>& nested_configs() const;
    std::vector<std::string>  FindSetting(const std::string& key) const;
    const ConfigParser& FindNesting(const std::string& key, int idx) const;

  private:
    std::string lvl_;
    std::string lvl_descr_;
    std::multimap<std::string, std::string> settings_;
    std::vector<ConfigParser> nested_configs_;
};

}  // namespace config

#endif  // WS_CONFIG_CONFIG_PARSER_H
