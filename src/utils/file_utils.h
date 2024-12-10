#ifndef WS_UTILS_FILE_UTILS_H
#define WS_UTILS_FILE_UTILS_H

#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

namespace utils {

bool DoesPathExist(const char *path) {
    return access(path, F_OK) != -1;
}

bool IsDirectory(const char *path) {
    struct stat info;

    if (stat(path, &info) != 0) {
        return false;
    }
    return (info.st_mode & S_IFDIR) != 0;
}


}  // namespace utils

#endif  // WS_UTILS_FILE_UTILS_H