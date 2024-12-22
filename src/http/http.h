#ifndef WS_HTTP_H
#define WS_HTTP_H

#include <maybe.h>

#include <string>

namespace http {

#define RQ_LINE_LEN_LIMIT 8192
#define RQ_TARGET_LEN_LIMIT 8192
// TODO rename, add unit, if its power of 2 then 1 << 15
#define RQ_HEADER_SECTION_LIMIT 32768
#define RQ_MAX_HEADER_COUNT 100

inline const char* kCRLF()
{
    return "\r\n";
}

enum Method {
    HTTP_NO_METHOD,
    HTTP_GET,
    HTTP_POST,
    HTTP_DELETE
};

enum Version {  // probably only need to handle Ver_1_0 and Ver_1_1
    HTTP_NO_VERSION,
    HTTP_0_9,
    HTTP_1_0,
    HTTP_1_1,
    HTTP_2,
    HTTP_3
};

std::string HttpVerToStr(http::Version ver);
std::string HttpMethodToStr(http::Method method);
utils::maybe<http::Version> HttpVersionFromStr(const std::string& version);
utils::maybe<http::Method> HttpMethodFromStr(const std::string& method);
utils::maybe<std::string> PercentDecode(const std::string& str, const char* dont_decode_set = NULL);
std::string PercentEncode(const std::string& str, const char* dont_encode_set = NULL);

static const char* kUnreserved =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-._~";
static const char* kGenDelims = ":/?#[]@";
static const char* kSubDelims = "!$&'()*+,;=";


}  // namespace http

#endif  // WS_HTTP_H
