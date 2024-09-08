#ifndef WS_HTTPCONFIGBUILDER_H
#define WS_HTTPCONFIGBUILDER_H

#include "AConfigBuilder.h"

class   HttpConfigBuilder : public AConfigBuilder {

    virtual const std::vector<std::string>  GetTokensByLvl() const;
  
  public:
    HttpConfigBuilder(std::ifstream& config_file, const std::string& lvl_descrt);
    ~HttpConfigBuilder();
    virtual utils::shared_ptr<IConfig>  Parse();
};

#endif  // WS_HTTPCONFIGBUILDER_H
