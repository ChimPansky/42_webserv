#include "SyntaxChecker.h"
#include "http.h"

#include <logger.h>
#include <cstring>

namespace http {

bool SyntaxChecker::CheckMethod(const std::string& method)
{
    for (size_t i = 0; i < method.size(); ++i) {
        if (std::strchr(kUnreserved, method[i]) == NULL) {
            return false;
        }
    }
    return true;
}

bool SyntaxChecker::CheckVersion(const std::string& version)
{
    for (size_t i = 0; i < version.size(); ++i) {
        if (std::strchr(kUnreserved, version[i]) == NULL && version[i] != '.' && version[i] != '/') {
            return false;
        }
    }
    return true;
}

bool SyntaxChecker::CheckHeaderKey(const std::string& header_name)
{
    (void)header_name;
    return true;
}

bool SyntaxChecker::CheckHeaderValue(const std::string& header_value)
{
    (void)header_value;
    return true;
}

}  // namespace http
