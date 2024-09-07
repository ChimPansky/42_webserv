#ifndef WS_ACONFIGBUILDER_H
#define WS_ACONFIGBUILDER_H

#include <string>
#include <vector>

#include "IConfig.h"
#include "utils/unique_ptr.h"

enum NestingLevel {
  NS_GLOBAL,
  NS_HTTP,
  NS_SERVER,
  NS_LOCATION
};

class   AConfigBuilder {

  protected:
    NestingLevel    lvl_;
    std::string     lvl_descrt_;
    std::vector<utils::unique_ptr<AConfigBuilder> > nested_builders_;
    typedef std::pair<std::string, std::string>  Setting;
    std::vector<Setting>    settings_;
    virtual NestingLevel    GetNestingByToken(const std::string& token) const = 0;
    virtual const std::vector<std::string>  GetTokensByLvl() const = 0;
    void    InitSettingsVector(std::ifstream& config_file, const std::string& lvl_descrt);
    Setting MakePair(const std::string& line);

  public:
    virtual ~AConfigBuilder();
    static utils::unique_ptr<AConfigBuilder>  GetBuilderByLvl(NestingLevel, std::ifstream&, const std::string&);
    virtual utils::unique_ptr<IConfig>  Parse() const = 0;
};

#endif  // WS_ACONFIGBUILDER_H
