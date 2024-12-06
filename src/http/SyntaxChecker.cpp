#include "SyntaxChecker.h"
#include "http.h"

#include <logger.h>
#include <cctype>
#include <cstring>
#include <algorithm>

namespace http {

bool SyntaxChecker::IsValidToken(const std::string& token) {
    if (token.empty()) {
        return false;
    }
    for (size_t i = 0; i < token.size(); ++i) {
        if (!IsTokenChar_(token[i])) {
            return false;
        }
    }
    return !token.empty(); // Token must not be empty
}

// https://datatracker.ietf.org/doc/html/rfc9112#name-method
// method         = token
bool SyntaxChecker::IsValidMethod(const std::string& method)
{
    return IsValidToken(method);
}

// https://datatracker.ietf.org/doc/html/rfc9112#name-http-version
// HTTP-version  = HTTP-name "/" DIGIT "." DIGIT
// HTTP-name     = %s"HTTP"
// actually: we really only can receive HTTP/1.0 or HTTP/1.1, anything below or above major version 1 doesn not even the version in the request line
 // example request line in 0.9: GET /path
bool SyntaxChecker::IsValidVersion(const std::string& version) {
    return version == "HTTP/1.0" || version == "HTTP/1.1";
}

// https://datatracker.ietf.org/doc/html/rfc9112#name-field-syntax
// field-name     = token
bool SyntaxChecker::IsValidHeaderKey(const std::string& header_name)
{
    (void)header_name;
    return true;
}

bool SyntaxChecker::IsValidHeaderValue(const std::string& header_value)
{
    (void)header_value;
    return true;
}

// utilities:
bool SyntaxChecker::IsTokenChar_(char c) {
    if (std::find(kTokenChars.begin(), kTokenChars.end(), c) != kTokenChars.end()) {
        return true;
    }
    return false;
}

}  // namespace http
