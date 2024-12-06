#include "SyntaxChecker.h"
#include "http.h"

#include <logger.h>
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
bool SyntaxChecker::IsValidVersion(const std::string& version) {
    const std::string httpName = "HTTP";
    // Check if version starts with "HTTP"
    if (version.compare(0, httpName.size(), httpName) != 0) {
        return false;
    }
    // Check if "/" follows "HTTP"
    size_t slashPos = httpName.size();
    if (slashPos >= version.size() || version[slashPos] != '/') {
        return false; // Missing or misplaced "/"
    }
    // Ensure the format after the slash is DIGIT "." DIGIT
    if (slashPos + 3 >= version.size()) {
        return false; // Not enough characters for DIGIT "." DIGIT
    }
    char majorDigit = version[slashPos + 1];
    char dot = version[slashPos + 2];
    char minorDigit = version[slashPos + 3];
    if (!IsDigit_(majorDigit) || dot != '.' || !IsDigit_(minorDigit)) {
        return false; // Incorrect format for DIGIT "." DIGIT
    }
    // Ensure no extra characters
    if (slashPos + 4 != version.size()) {
        return false; // Extra characters after DIGIT "." DIGIT
    }
    return true; // All checks passed
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
bool SyntaxChecker::IsDigit_(char c) {
    return c >= '0' && c <= '9';
}

bool SyntaxChecker::IsTokenChar_(char c) {
    if (std::find(kTokenChars.begin(), kTokenChars.end(), c) != kTokenChars.end()) {
        return true;
    }
    return false;
}

}  // namespace http
