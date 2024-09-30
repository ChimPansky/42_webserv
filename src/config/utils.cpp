#include "config/utils.h"

#include <climits>
#include <cstddef>
#include <cstdlib>
#include <fstream>
#include <iostream>
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
        if (!line.substr(start, end - start).empty()) {
            elements.push_back(line.substr(start, end - start));
        }
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
    return file.length() > extention.length() && (file.find_last_of('.') != std::string::npos &&
                                                  file.substr(file.find_last_of('.')) == extention);
}

bool ValidPath(const std::string& val)
{
    if (val.empty() || val[0] != '/') {
        return false;
    }

    std::ofstream file(val.substr(1).c_str(), std::ios::out);
    if (!file.is_open()) {
        return false;
    }
    return true;
}

bool IsDirectory(const std::string& path)
{
    DIR* dir;
    if (path.empty() || path[0] != '/') {
        return false;
    } else {
        dir = opendir(path.substr(1).c_str());
    }

    if (dir) {
        closedir(dir);
        return true;
    } else {
        return false;
    }
}

std::string Trim(const std::string& str, const std::string& trim_chars)
{
    std::string trimmed = str;
    if (str.find_last_not_of(trim_chars) == std::string::npos) {
        return std::string();
    }
    trimmed.erase(trimmed.find_last_not_of(trim_chars) + 1);
    return trimmed;
}

}  // namespace config
