#ifndef WS_HTTP_H
#define WS_HTTP_H

#include <string>

namespace http {

#define RQ_LINE_LEN_LIMIT 8192
#define RQ_TARGET_LEN_LIMIT 8192
#define RQ_MAX_HEADER_COUNT 100

inline const char* kCRLF() {return "\r\n";}

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

std::pair<bool/*found*/, std::string/*version*/> HttpVerToStr(http::Version ver);
std::pair<bool/*found*/, std::string/*method*/> HttpMethodToStr(http::Method method);
std::pair<bool/*found*/, http::Version> HttpVersionFromStr(const std::string& version);
std::pair<bool/*found*/, http::Method> HttpMethodFromStr(const std::string& method);

static const char* kUnreserved = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-._~";
static const char* kGenDelims = ":/?#[]@";
static const char* kSubDelims = "!$&'()*+,;=";


}  // namespace http

#endif  // WS_HTTP_H
