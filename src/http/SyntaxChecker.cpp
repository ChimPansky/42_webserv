#include "SyntaxChecker.h"

#include <logger.h>

namespace http {

// token = 1*tchar
bool SyntaxChecker::IsValidTokenName(const std::string& token)
{
    if (token.empty()) {
        return false;
    }
    for (size_t i = 0; i < token.size(); ++i) {
        if (!IsTokenChar_(token[i])) {
            return false;
        }
    }
    return !token.empty();  // Token must not be empty
}

// https://datatracker.ietf.org/doc/html/rfc9112#name-method
// method         = token
bool SyntaxChecker::IsValidMethodName(const std::string& method)
{
    return IsValidTokenName(method);
}

// https://datatracker.ietf.org/doc/html/rfc9112#name-http-version
// HTTP-version  = HTTP-name "/" DIGIT "." DIGIT
// HTTP-name     = %s"HTTP"
// actually: we really only can receive HTTP/1.0 or HTTP/1.1, anything below or above major version
// 1 doesn not even the version in the request line example request line in 0.9: GET /path
bool SyntaxChecker::IsValidVersionName(const std::string& version)
{
    return version == "HTTP/1.0" || version == "HTTP/1.1";
}

// https://datatracker.ietf.org/doc/html/rfc9112#name-field-syntax
// field-name     = token
bool SyntaxChecker::IsValidHeaderKeyName(const std::string& header_name)
{
    return IsValidTokenName(header_name);
}

// https://datatracker.ietf.org/doc/html/rfc9110#name-field-values
// field-value    = *field-content
//   field-content  = field-vchar
//                    [ 1*( SP / HTAB / field-vchar ) field-vchar ]
//   field-vchar    = VCHAR / obs-text
//   obs-text       = %x80-FF
bool SyntaxChecker::IsValidHeaderValueName(const std::string& header_value)
{
    size_t len = header_value.size();
    size_t i = 0;
    if (len > 0 && !IsFieldVChar_(header_value[i])) {
        return false;
    }
    while (i < len) {
        if (!IsWhiteSpace_(header_value[i]) && !IsFieldVChar_(header_value[i])) {
            return false;
        }
        ++i;
    }
    return true;
}

// utilities:
// tchar = "!" / "#" / "$" / "%" / "&" / "'" / "*" / "+" / "-" / "." /
//  "^" / "_" / "`" / "|" / "~" / DIGIT / ALPHA
bool SyntaxChecker::IsTokenChar_(char c)
{
    if (std::isalnum(c)) {
        return true;
    }
    const std::string specialTChars = "!#$%&'*+-.^_`|~";
    return specialTChars.find(c) != std::string::npos;
}

// visible (printing) characters
// VCHAR =  %x21-7E
bool SyntaxChecker::IsVChar_(char c)
{
    return std::isprint(c) && c != ' ';
}

// obs-text = %x80-FF
bool SyntaxChecker::IsObsText_(char c)
{
    return (static_cast<unsigned char>(c) >= 128 && static_cast<unsigned char>(c) <= 255);
}

// field-vchar = VCHAR / obs-text
bool SyntaxChecker::IsFieldVChar_(char c)
{
    return (IsVChar_(c) || IsObsText_(c));
}

// WSP = SP / HTAB
bool SyntaxChecker::IsWhiteSpace_(char c)
{
    return (c == ' ' || c == '\t');
}

}  // namespace http
