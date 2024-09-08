#ifndef WS_CONFIGBUILDER_H
#define WS_CONFIGBUILDER_H

#include "fstream"
#include "AConfigBuilder.h"

class   ConfigBuilder : public AConfigBuilder {

    virtual const std::vector<std::string>  GetTokensByLvl() const;
  
  public:
    ConfigBuilder(std::ifstream& config_file, const std::string& lvl_descrt);
    ~ConfigBuilder();
    static  ConfigBuilder StartBuilder(const std::string& config_path);
    virtual utils::shared_ptr<IConfig>  Parse();
};

#endif  // WS_CONFIGBUILDER_H
