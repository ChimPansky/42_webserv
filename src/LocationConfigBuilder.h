#ifndef WS_LOCATIONBUILDER_H
#define WS_LOCATIONBUILDER_H

#include "IConfigBuilder.h"

class   LocationBuilder : public IConfigBuilder {

    virtual std::vector<std::string>  GetTokensByLvl(const std::string& token) const = 0;
  
  public:
    ~LocationBuilder();
    static utils::unique_ptr<IConfigBuilder>  GetBuilderByLvl(NestingLevel lvl);
    virtual utils::unique_ptr<IConfig>  Parse() const;
};

#endif  // WS_LOCATIONBUILDER_H
