#include "http.h"
#include <sstream>

namespace http {

std::string UnsignedShortToStr(unsigned short num) {
    std::stringstream ss;
    ss << num;
    return ss.str();
}

} // namespace http
