#ifndef WS_HTTP_H
#define WS_HTTP_H

namespace http {

#define RQ_LINE_LEN_LIMIT 512 // todo: increase this later to 8192. keep it low for testing purposes
#define RQ_TARGET_LEN_LIMIT 256 // todo: increase this later. keep it low for testing purposes

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

enum EOL_CHARS {
    EOL_CARRIAGE_RETURN = '\r',
    EOL_LINE_FEED = '\n'
};

const char* HttpVerToStr(http::Version ver);

}  // namespace http

#endif  // WS_HTTP_H
