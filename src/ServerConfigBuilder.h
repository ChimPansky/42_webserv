#ifndef WS_HTTPCONFIGBUILDER_H
#define WS_HTTPCONFIGBUILDER_H

#include "IConfigBuilder.h"

class   ServerConfigBuilder : public IConfigBuilder {

    virtual std::vector<std::string>  GetTokensByLvl(const std::string& token) const = 0;
  
  public:
    ~ServerConfigBuilder();
    static utils::unique_ptr<IConfigBuilder>  GetBuilderByLvl(NestingLevel lvl);
    virtual utils::unique_ptr<IConfig>  Parse() const;
};

#endif  // WS_HTTPCONFIGBUILDER_H
