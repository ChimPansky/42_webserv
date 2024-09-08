#ifndef WS_LOCATIONCONFIGBUILDER_H
#define WS_LOCATIONCONFIGBUILDER_H

#include "AConfigBuilder.h"

class   LocationConfigBuilder : public AConfigBuilder {

    virtual const std::vector<std::string>  GetTokensByLvl() const;
  
  public:
    LocationConfigBuilder(std::ifstream& config_file, const std::string& lvl_descrt);
    ~LocationConfigBuilder();
    virtual utils::shared_ptr<IConfig>  Parse();
};

#endif  // WS_LOCATIONCONFIGBUILDER_H
