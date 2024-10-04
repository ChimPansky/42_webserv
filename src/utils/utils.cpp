#include "utils.h"

namespace utils {

std::string ToLowerCase(const std::string& input)
{
    std::string result = input;
    for (size_t i = 0; i < result.size(); i++) {
        result[i] = std::tolower(result[i]);
    }
    return result;
}

std::pair<bool /* is_valid*/, size_t> HexStrToSizeT_(const std::string& hex_str)
{
    size_t result = 0;
    char c = 0;
    if (hex_str.size() == 0) {
        return std::make_pair(false, 0);
    }
    for (size_t i = 0; i < hex_str.size(); i++) {
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
