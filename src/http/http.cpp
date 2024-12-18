#include "http.h"

#include <cstring>

#include "logger.h"
#include "numeric_utils.h"

namespace http {

std::pair<bool /*found*/, std::string /*version*/> HttpVerToStr(Version ver)
{
    switch (ver) {
        case HTTP_0_9: return std::make_pair(true, "HTTP/0.9");
        case HTTP_1_0: return std::make_pair(true, "HTTP/1.0");
        case HTTP_1_1: return std::make_pair(true, "HTTP/1.1");
        case HTTP_2: return std::make_pair(true, "HTTP/2");
        case HTTP_3: return std::make_pair(true, "HTTP/3");
        default: return std::make_pair(false, "Version not found");
    }
}

std::pair<bool /*found*/, std::string /*method*/> HttpMethodToStr(Method method)
{
    switch (method) {
        case HTTP_GET: return std::make_pair(true, "GET");
        case HTTP_POST: return std::make_pair(true, "POST");
        case HTTP_DELETE: return std::make_pair(true, "DELETE");
        default: return std::make_pair(false, "Method not found");
    }
}

std::pair<bool /*found*/, http::Method> HttpMethodFromStr(const std::string& raw_method)
{
    if (raw_method == "GET") {
        return std::make_pair(true, HTTP_GET);
    } else if (raw_method == "POST") {
        return std::make_pair(true, HTTP_POST);
    } else if (raw_method == "DELETE") {
        return std::make_pair(true, HTTP_DELETE);
    } else {
        return std::make_pair(false, HTTP_NO_METHOD);
    }
}

std::pair<bool /*found*/, http::Version> HttpVersionFromStr(const std::string& raw_version)
{
    if (raw_version == "HTTP/0.9") {
        return std::make_pair(true, HTTP_0_9);
    } else if (raw_version == "HTTP/1.0") {
        return std::make_pair(true, HTTP_1_0);
    } else if (raw_version == "HTTP/1.1") {
        return std::make_pair(true, HTTP_1_1);
    } else if (raw_version == "HTTP/2") {
        return std::make_pair(true, HTTP_2);
    } else if (raw_version == "HTTP/3") {
        return std::make_pair(true, HTTP_3);
    } else {
        LOG(INFO) << "HttpVersionFromStr: DID NOT MATCH ANY VERSION";
        return std::make_pair(false, HTTP_NO_VERSION);
    }
}

// decode all percent-encoded characters, except those in the dont_decode_set
std::pair<bool /*decoding_successful*/, std::string /*decoded_str*/> PercentDecode(
    const std::string& str, const char* dont_decode_set)
{
    std::string decoded;
    std::pair<bool, unsigned short> ascii;
    for (size_t i = 0; i < str.size(); ++i) {
        if (str[i] == '%') {
            if (i + 2 >= str.size()) {
                return std::pair<bool, std::string>(false, "");
            }
            ascii = utils::HexToUnsignedNumericNoThrow<unsigned short>(str.substr(i + 1, 2));
            if (dont_decode_set && strchr(dont_decode_set, static_cast<char>(ascii.second))) {
                decoded += str.substr(i, 3);
            } else {
                if (!ascii.first) {
                    return std::pair<bool, std::string>(false, "");
                }
                decoded += static_cast<char>(ascii.second);
            }
            i += 2;
        } else {
            decoded += str[i];
        }
    }
    return std::pair<bool, std::string>(true, decoded);
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
