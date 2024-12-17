#ifndef WS_UTILS_FILE_UTILS_H
#define WS_UTILS_FILE_UTILS_H

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cstdio>
#include <fstream>
#include <string>

namespace utils {

const static int kMaxTempFileName = TMP_MAX;

bool DoesPathExist(const char *path);

bool IsReadable(const char *path);

bool IsExecutable(const char *path);

bool IsDirectory(const char *path);

std::pair<bool /*success*/, std::string /*file_content*/> ReadFileToString(const char *filePath);

bool CheckFileExtension(const std::string &file, const std::string &extention);

template <class FileStream>
bool CreateAndOpenTmpFileToStream(FileStream &fs, char *tmp_file_path)
{
    if (!std::tmpnam(tmp_file_path)) {
        return false;
    }
    // Create and use the file
    fs.open(tmp_file_path);
    return fs.is_open();
}

}  // namespace utils

#endif  // WS_UTILS_FILE_UTILS_H
