#ifndef WS_SERVER_UTILS_UTILS_H
#define WS_SERVER_UTILS_UTILS_H

#include <map>
#include <string>

namespace utils {

std::map<const char*, const char*> GetMimeTypes();
inline const char* kDefaultContentType()
{
    return "application/octet-stream";
}

}  // namespace utils

#endif  // WS_SERVER_UTILS_UTILS_H
