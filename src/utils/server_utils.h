#ifndef WS_UTILS_SERVER_UTILS_H
#define WS_UTILS_SERVER_UTILS_H

#include <map>
#include <string>

namespace utils {
namespace serv {

std::map<std::string, std::string> GetMimeTypes();
inline const char* kDefaultContentType()
{
    return "application/octet-stream";
}

}  // namespace serv
}  // namespace utils

#endif  // WS_UTILS_SERVER_UTILS_H
