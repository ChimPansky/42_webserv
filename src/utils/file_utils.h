#ifndef WS_UTILS_FILE_UTILS_H
#define WS_UTILS_FILE_UTILS_H

#include <fcntl.h>
#include <string>
#include <sys/stat.h>
#include <unistd.h>

namespace utils {

bool DoesPathExist(const char *path);
bool IsDirectory(const char *path);
std::pair<bool/*success*/, std::string/*file_content*/> ReadFileToString(const char* filePath);

}  // namespace utils

#endif  // WS_UTILS_FILE_UTILS_H