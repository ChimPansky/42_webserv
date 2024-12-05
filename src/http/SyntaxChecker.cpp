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

}  // namespace http
