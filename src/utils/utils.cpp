#include "utils.h"

namespace utils {

std::string ToLowerCase(std::string str)
{
    for (size_t i = 0; i < str.size(); i++) {
        str[i] = std::tolower(str[i]);
    }
    return str;
}

}  // namespace utils
