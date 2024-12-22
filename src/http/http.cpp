#include "http.h"

#include <logger.h>
#include <maybe.h>
#include <numeric_utils.h>

#include <cstring>

namespace http {

std::string HttpVerToStr(Version ver)
{
    switch (ver) {
        case HTTP_0_9: return std::string("HTTP/0.9");
        case HTTP_1_0: return std::string("HTTP/1.0");
        case HTTP_1_1: return std::string("HTTP/1.1");
        case HTTP_2: return std::string("HTTP/2");
        case HTTP_3: return std::string("HTTP/3");
        default: return "UNKNOWN VERSION";
    }
}

std::string HttpMethodToStr(Method method)
{
    switch (method) {
        case HTTP_GET: return std::string("GET");
        case HTTP_POST: return std::string("POST");
        case HTTP_DELETE: return std::string("DELETE");
        default: return "UNKNOWN METHOD";
    }
}

utils::maybe<http::Method> HttpMethodFromStr(const std::string& raw_method)
{
    if (raw_method == "GET") {
        return HTTP_GET;
    } else if (raw_method == "POST") {
        return HTTP_POST;
    } else if (raw_method == "DELETE") {
        return HTTP_DELETE;
    } else {
        return utils::maybe_not();
    }
}

utils::maybe<http::Version> HttpVersionFromStr(const std::string& raw_version)
{
    if (raw_version == "HTTP/0.9") {
        return HTTP_0_9;
    } else if (raw_version == "HTTP/1.0") {
        return HTTP_1_0;
    } else if (raw_version == "HTTP/1.1") {
        return HTTP_1_1;
    } else if (raw_version == "HTTP/2") {
        return HTTP_2;
    } else if (raw_version == "HTTP/3") {
        return HTTP_3;
    } else {
        LOG(INFO) << "HttpVersionFromStr: DID NOT MATCH ANY VERSION";
        return utils::maybe_not();
    }
}

// decode all percent-encoded characters, except those in the dont_decode_set
utils::maybe<std::string> PercentDecode(const std::string& str, const char* dont_decode_set)
{
    std::string decoded;
    for (size_t i = 0; i < str.size(); ++i) {
        if (str[i] == '%') {
            if (i + 2 >= str.size()) {
                return utils::maybe_not();
            }
            utils::maybe<unsigned short> ascii =
                utils::HexToUnsignedNumericNoThrow<unsigned short>(str.substr(i + 1, 2));
            if (!ascii.ok()) {
                return utils::maybe_not();
            }
            if (dont_decode_set && strchr(dont_decode_set, static_cast<char>(*ascii))) {
                decoded += str.substr(i, 3);
            } else {
                decoded += static_cast<char>(*ascii);
            }
            i += 2;
        } else {
            decoded += str[i];
        }
    }
    return decoded;
}

// encode all chars that are not in the unreserved set (delimiters, whitespaces, etc.) and not in
// the dont_encode_set for example: dont encode "/" in path, but encode it in query
std::string PercentEncode(const std::string& str, const char* dont_encode_set)
{
    std::string encoded;
    for (size_t i = 0; i < str.size(); ++i) {
        if (strchr(kUnreserved, str[i]) == NULL &&
            (dont_encode_set && strchr(dont_encode_set, str[i]) == NULL)) {
            encoded += '%';
            encoded += utils::NumericToHexStr(str[i]);
        } else {
            encoded += str[i];
        }
    }
    return encoded;
}

}  // namespace http
