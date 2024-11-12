#ifndef WS_HTTP_H
#define WS_HTTP_H

namespace http {

#define RQ_LINE_LEN_LIMIT 8192
#define RQ_URI_LEN_LIMIT 10

inline const char* LineSep() {return "\r\n";}

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

const char* HttpVerToStr(http::Version ver);

}  // namespace http

#endif  // WS_HTTP_H
