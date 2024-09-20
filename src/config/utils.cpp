#include "config/utils.h"
#include <iostream>

namespace config {

std::vector<std::string> SplitLine(const std::string& line) {

    std::vector<std::string>    elements;
    size_t  start = 0;
    size_t  end = 0;
    while (end != std::string::npos) {
        end = line.find(' ', start);
        elements.push_back(line.substr(start, end - start));
        start = end + 1;
    }
    return elements;
}

std::pair<std::string, std::string> MakePair(const std::string& line)
{
    size_t  pos = line.find_first_of(" \t");
    if (pos == std::string::npos) {
        return std::make_pair(line, "");
    }
    size_t  start = line.find_first_not_of(" \t", pos);
    if (start == std::string::npos) {
        return std::make_pair(line.substr(0, pos), "");
    }
    return std::make_pair(line.substr(0, pos), line.substr(start, line.size() - start));
}

bool    CheckFileExtension(const std::string& file, const std::string& extention)
{
    return file.length() > extention.length() || (file.find_last_of('.') != std::string::npos && file.substr(file.find_last_of('.')) == extention);
}

} // namespace config
