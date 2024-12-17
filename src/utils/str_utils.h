#ifndef WS_UTILS_STR_UTILS_H
#define WS_UTILS_STR_UTILS_H

#include <string>
#include <vector>

namespace utils {

std::string ToLowerCase(std::string str);
void EatSpacesAndHTabs(std::stringstream& ss);
std::string Trim(const std::string& str, const std::string& trim_chars);
std::vector<std::string> SplitLine(const std::string& line);

}  // namespace utils

#endif  // WS_UTILS_STR_UTILS_H
