#ifndef WS_SERVER_UTILS_UTILS_H
#define WS_SERVER_UTILS_UTILS_H

#include <string>

namespace utils {

const char* GetTypeByExt(const std::string& ext);
std::string GetInterpreterByExt(const std::string& filename);
inline const char* kDefaultContentType()
{
    return "application/octet-stream";
}

}  // namespace utils

#endif  // WS_SERVER_UTILS_UTILS_H
