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

}  // namespace utils
