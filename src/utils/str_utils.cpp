#include "str_utils.h"

#include <dirent.h>

#include <fstream>
#include <sstream>

namespace utils {

std::string ToLowerCase(std::string str)
{
    for (size_t i = 0; i < str.size(); i++) {
        str[i] = std::tolower(str[i]);
    }
    return str;
}

void EatSpacesAndHTabs(std::stringstream& ss)
{
    while (ss.peek() == ' ' || ss.peek() == '\t') {
        ss.ignore();
    }
}

std::string Trim(const std::string& str, const std::string& trim_chars = " \t")
{
    size_t first = str.find_first_not_of(trim_chars);
    if (first == std::string::npos) {
        return std::string();
    }
    size_t last = str.find_last_not_of(trim_chars);
    return str.substr(first, last - first + 1);
}

std::vector<std::string> SplitLine(const std::string& line)
{
    std::vector<std::string> elements;
    size_t start = 0;
    size_t end = 0;
    while (start < line.size()) {
        end = line.find(' ', start);
        if (end == std::string::npos) {
            end = line.size();
        }
        if (!line.substr(start, end - start).empty()) {
            elements.push_back(line.substr(start, end - start));
        }
        start = end + 1;
    }
    return elements;
}

}  // namespace utils
