#ifndef WS_SERVERCONFIGBUILDER_H
#define WS_SERVERCONFIGBUILDER_H

#include "AConfigBuilder.h"

class   ServerConfigBuilder : public AConfigBuilder {

    virtual const std::vector<std::string>  GetTokensByLvl() const;
  
  public:
    ServerConfigBuilder(std::ifstream& config_file, const std::string& lvl_descrt);
    ~ServerConfigBuilder();
    virtual utils::shared_ptr<IConfig>  Parse();
};

#endif  // WS_SERVERCONFIGBUILDER_H
