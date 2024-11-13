#include "http.h"
#include <stdexcept>

const char* http::HttpVerToStr(Version ver) {
    switch (ver) {
        case http::HTTP_0_9: return "HTTP/0.9";
        case http::HTTP_1_0: return "HTTP/1.0";
        case http::HTTP_1_1: return "HTTP/1.1";
        case http::HTTP_2: return "HTTP/2";
        case http::HTTP_3: return "HTTP/3";
        default: throw std::logic_error("attempt to get a name of unknown http ver");
    }
}