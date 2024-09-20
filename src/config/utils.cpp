#include "config/utils.h"

#include <climits>
#include <cstdlib>
#include <stdexcept>
#include <string>

namespace config {

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
        elements.push_back(line.substr(start, end - start));
        start = end + 1;
    }
    return elements;
}

std::pair<std::string, std::string> MakePair(const std::string& line)
{
    size_t pos = line.find_first_of(" \t");
    if (pos == std::string::npos) {
        return std::make_pair(line, "");
    }
    size_t start = line.find_first_not_of(" \t", pos);
    if (start == std::string::npos) {
        return std::make_pair(line.substr(0, pos), "");
    }
    return std::make_pair(line.substr(0, pos), line.substr(start, line.size() - start));
}

bool CheckFileExtension(const std::string& file, const std::string& extention)
{
    return file.length() > extention.length() || (file.find_last_of('.') != std::string::npos &&
                                                  file.substr(file.find_last_of('.')) == extention);
}

int StrToInt(const std::string& str)
{
    long value = std::atol(str.c_str());

    // Check for underflow and overflow
    if (value < INT_MIN || value > INT_MAX) {
        throw std::runtime_error("Integer conversion out of range");
    }
    return static_cast<int>(value);
}

}  // namespace config
