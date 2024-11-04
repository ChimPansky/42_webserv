#ifndef WS_UTILS_UTILS_H
#define WS_UTILS_UTILS_H

#include <dirent.h>
#include <netdb.h>


#include <string>
#include <vector>
#include <ctime>

namespace utils {

std::string ToLowerCase(std::string str);

namespace fs {

std::vector<std::string> SplitLine(const std::string& line);
bool CheckFileExtension(const std::string& file, const std::string& extention);
bool ValidPath(const std::string& val);
bool IsDirectory(const std::string& path);
std::string Trim(const std::string& str, const std::string& trim_chars);

}  // namespace fs

}  // namespace utils

#endif  // WS_UTILS_UTILS_H
