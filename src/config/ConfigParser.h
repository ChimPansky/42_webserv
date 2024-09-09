#ifndef WS_PARSED_CONFIG_FILE_H
#define WS_PARSED_CONFIG_FILE_H

#include <fstream>
#include <string>
#include <map>
#include <vector>

enum NestingLevel {
  NS_GLOBAL,
  NS_HTTP,
  NS_SERVER,
  NS_LOCATION
};

class ConfigParser  {

  public:
    ConfigParser(std::ifstream& ifs, const std::string& lvl, const std::string& lvl_descrt = "");
    typedef std::pair<std::string, std::string>  Setting;
    Setting MakePair(const std::string& line);
    
    // NestingLevel    GetNestingByToken(const std::string& token);

  // public:
    // static utils::unique_ptr<ConfigParser>  GetBuilderByLvl(NestingLevel, std::ifstream&, const std::string&);

    const std::string& lvl() const {return lvl_;}
    const std::string& lvl_descr() const {return lvl_descr_;}
    const std::multimap<std::string, std::string>& settings() const {return settings_;}
    const std::vector<ConfigParser>& nested_configs() const {return nested_configs_;}
    static ConfigParser MakeParser(const std::string& config_path);

    std::vector<const std::string&> FindSettings(const std::string& key) {
      std::vector<const std::string&> res;
      for (std::multimap<std::string, std::string>::const_iterator it = settings_.begin(); it != settings_.end(); ++it) {
        if (it->first == key) {
          res.push_back(it->second);
        }
      }
      return res;
    }

  private:
    std::string     lvl_;
    std::string     lvl_descr_;
    std::multimap<std::string, std::string>    settings_;
    std::vector<ConfigParser> nested_configs_;
    // typedef std::vector<Setting>::const_iterator SettingIt;
    // typedef std::vector<ConfigParser>::const_iterator NestedConfigIt;
};

#endif  // WS_PARSED_CONFIG_FILE_H
