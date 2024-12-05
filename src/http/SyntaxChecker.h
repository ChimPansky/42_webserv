#ifndef WS_HTTP_SYNTAX_CHECKER_H
#define WS_HTTP_SYNTAX_CHECKER_H


#include <string>
namespace http {

class SyntaxChecker {
  private:
    SyntaxChecker();

  public:
    static bool CheckMethod(const std::string& method);
    static bool CheckVersion(const std::string& version);
    static bool CheckHeaderKey(const std::string& header_name);
    static bool CheckHeaderValue(const std::string& header_value);
};

}  // namespace http

#endif  // WS_HTTP_SYNTAX_CHECKER_H
