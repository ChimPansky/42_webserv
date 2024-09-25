#include "config/utils.h"

#include <climits>
#include <cstddef>
#include <cstdlib>
#include <fstream>
#include <iostream>
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

int StrToInt(const std::string& str)
{
    if (str.find_first_not_of("0123456789") != std::string::npos) {
        throw std::runtime_error("Nonnumeric characters in string");
    }
    long value = std::atol(str.c_str());

    if (value < INT_MIN || value > INT_MAX) {
        throw std::runtime_error("Integer conversion out of range");
    }
    return static_cast<int>(value);
}

size_t StrToUnsignedInt(const std::string& str)
{
    if (str.find_first_not_of("0123456789") != std::string::npos) {
        throw std::runtime_error("Nonnumeric characters in string");
    }
    long value = std::atol(str.c_str());

    if (value < 0 || value > UINT_MAX) {
        throw std::runtime_error("Negative value cannot be converted to unsigned int");
    }
    return static_cast<size_t>(value);
}

in_port_t   StrToInPortT(const std::string& str) {
    if (str.find_first_not_of("0123456789") != std::string::npos) {
        throw std::runtime_error("Nonnumeric characters in string");
    }
    long value = std::atol(str.c_str());

    if (value < 0 || value > 65535) {
        throw std::runtime_error("in_port_t conversion out of range");
    }

    return static_cast<in_port_t>(value);
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
    if (!path.empty() && path[0] == '/') {
        dir = opendir(path.substr(1).c_str());
    } else {
        dir = opendir(path.c_str());
    }

    if (dir) {
        closedir(dir);
        return true;
    } else {
        return false;
    }
}

std::string  Trim(const std::string& str, const std::string& trim_chars) {

    std::string trimmed = str;
    if (str.find_last_not_of(trim_chars) == std::string::npos) {
        return std::string();
    }
    trimmed.erase(trimmed.find_last_not_of(trim_chars) + 1);
    return trimmed;
}

}  // namespace config
