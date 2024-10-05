#include "utils.h"
#include <limits>

namespace http {

std::pair<bool /* is_valid*/, size_t> HexStrToSizeT(const std::string& hex_str)
{
    size_t result = 0;
    char c = 0;
    if (hex_str.empty()) {
        return std::make_pair(false, 0);
    }
    for (size_t i = 0; i < hex_str.size(); i++) {
        if (result > std::numeric_limits<size_t>::max() >> 4) {
            return std::make_pair(false, 0);
        }
        result *= 16;
        c = std::tolower(hex_str[i]);
        if (std::isdigit(c)) {
            result += c - '0';
        } else if (c >= 'a' && c <= 'f') {
            result += c - 'a' + 10;
        } else {
            return std::make_pair(false, 0);
        }
    }
    return std::make_pair(true, result);
}

}  // namespace utils
