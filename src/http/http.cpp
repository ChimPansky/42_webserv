#include "http.h"
#include <stdexcept>

namespace http {

const char* HttpVerToStr(Version ver) {
    switch (ver) {
        case HTTP_0_9: return "HTTP/0.9";
        case HTTP_1_0: return "HTTP/1.0";
        case HTTP_1_1: return "HTTP/1.1";
        case HTTP_2: return "HTTP/2";
        case HTTP_3: return "HTTP/3";
        default: throw std::logic_error("attempt to get a name of unknown http ver");
    }
}
}  // namespace http