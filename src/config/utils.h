#ifndef WS_CONFIG_UTILS_H
# define WS_CONFIG_UTILS_H

#include <string>
#include <vector>

namespace config {

std::vector<std::string> SplitLine(const std::string& line);
std::pair<std::string, std::string> MakePair(const std::string& line);
bool    CheckFileExtension(const std::string& file, const std::string& extention);

}

#endif // WS_CONFIG_UTILS_H
