#ifndef WS_CONFIG_UTILS_H
#define WS_CONFIG_UTILS_H

#include <dirent.h>

#include <string>
#include <vector>

namespace config {

std::vector<std::string> SplitLine(const std::string& line);
std::pair<std::string, std::string> MakePair(const std::string& line);
bool CheckFileExtension(const std::string& file, const std::string& extention);
int StrToInt(const std::string& str);
size_t StrToUnsignedInt(const std::string& str);
bool ValidPath(const std::string& val);
bool IsDirectory(const std::string& path);

}  // namespace config

#endif  // WS_CONFIG_UTILS_H
