#ifndef WS_ACONFIGBUILDER_H
#define WS_ACONFIGBUILDER_H

#include <fstream>
#include <string>
#include <vector>

#include "IConfig.h"
#include "utils/unique_ptr.h"
#include "utils/shared_ptr.h"

enum NestingLevel {
  NS_GLOBAL,
  NS_HTTP,
  NS_SERVER,
  NS_LOCATION
};

class   AConfigBuilder {

  protected:
    typedef std::pair<std::string, std::string>  Setting;
    AConfigBuilder(std::ifstream& ifs, const std::string& lvl_descrt);
    NestingLevel    GetNestingByToken(const std::string& token);
    virtual const std::vector<std::string>  GetTokensByLvl() const = 0;
    Setting MakePair(const std::string& line);

  public:
    virtual ~AConfigBuilder();
    static utils::unique_ptr<AConfigBuilder>  GetBuilderByLvl(NestingLevel, std::ifstream&, const std::string&);
    virtual utils::shared_ptr<IConfig>  Parse() = 0;

  protected:
    NestingLevel    lvl_;
    std::string     lvl_descrt_;
    std::vector<Setting>    settings_;
    std::vector<utils::unique_ptr<AConfigBuilder> > nested_builders_;
};

#endif  // WS_ACONFIGBUILDER_H
