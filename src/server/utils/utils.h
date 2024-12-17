#ifndef WS_SERVER_UTILS_UTILS_H
#define WS_SERVER_UTILS_UTILS_H

#include <map>
#include <string>

namespace utils {

const char* GetTypeByExt(const std::string& ext);
std::string GetInterpreterByExt(const std::string& filename);
inline const char* kDefaultContentType()
{
    return "application/octet-stream";
}

std::string UpdatePath(const std::string& loc, const std::string& matched_prefix,
                       const std::string& uri_path);

}  // namespace utils

#endif  // WS_SERVER_UTILS_UTILS_H
