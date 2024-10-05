#include "utils.h"
#include <fstream>

namespace utils {

std::string ToLowerCase(std::string str)
{
    for (size_t i = 0; i < str.size(); i++) {
        str[i] = std::tolower(str[i]);
    }
    return str;
}

namespace fs {

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

bool CheckFileExtension(const std::string& file, const std::string& extention)
{
    if (file.length() > extention.length() && file[file.size() - 6] == '/') {
        return false;
    }
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

std::string Trim(const std::string& str, const std::string& trim_chars = " \t")
{
    std::string trimmed = str;
    if (str.find_last_not_of(trim_chars) == std::string::npos) {
        return std::string();
    }
    trimmed.erase(trimmed.find_last_not_of(trim_chars) + 1);
    return trimmed;
}

}  // namespace fs

}  // namespace utils
