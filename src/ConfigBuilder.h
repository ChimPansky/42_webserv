#ifndef WS_CONFIGBUILDER_H
#define WS_CONFIGBUILDER_H

#include <iostream>
#include "fstream"
#include "AConfigBuilder.h"

class   ConfigBuilder : public AConfigBuilder {

    virtual const std::vector<std::string>  GetTokensByLvl() const;
  
  public:
    ConfigBuilder(std::ifstream& config_file);
    ~ConfigBuilder();
    static  ConfigBuilder StartBuilder(const std::string& config_path);
    static utils::unique_ptr<AConfigBuilder>  GetBuilderByLvl(NestingLevel lvl);
    virtual NestingLevel    GetNestingByToken(const std::string& token) const;
    virtual utils::unique_ptr<IConfig>  Parse() const;
};

#endif  // WS_CONFIGBUILDER_H
