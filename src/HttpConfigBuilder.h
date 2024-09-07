#ifndef WS_HTTPBUILDER_H
#define WS_HTTPBUILDER_H

#include "IConfigBuilder.h"

class   HttpBuilder : public IConfigBuilder {

    virtual std::vector<std::string>  GetTokensByLvl(const std::string& token) const = 0;
  
  public:
    ~HttpBuilder();
    static utils::unique_ptr<IConfigBuilder>  GetBuilderByLvl(NestingLevel lvl);
    virtual utils::unique_ptr<IConfig>  Parse() const;
};

#endif  // WS_HTTPBUILDER_H
