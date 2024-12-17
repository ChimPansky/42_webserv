#include "file_utils.h"

#include <fstream>
#include <sstream>

#include "logger.h"

namespace utils {

bool DoesPathExist(const char *path)
{
    return access(path, F_OK) != -1;
}

bool IsReadable(const char *path)
{
    return access(path, R_OK) != -1;
}

bool IsExecutable(const char *path)
{
    return access(path, X_OK) != -1;
}

bool IsDirectory(const char *path)
{
    struct stat info;

    if (stat(path, &info) != 0) {
        return false;
    }
    return (info.st_mode & S_IFDIR) != 0;
}

std::pair<bool /*success*/, std::string /*file_content*/> ReadFileToString(const char *filePath)
{
    if (!filePath[0]) {
        LOG(ERROR) << "Empty file path";
        return std::make_pair(false, "");
    }
    std::ifstream file(filePath);
    if (!file.is_open()) {
        LOG(ERROR) << "utils::ReadFileToString: Could not open file: " << filePath;
        return std::make_pair(false, "");
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return std::make_pair(true, buffer.str());
}

bool CheckFileExtension(const std::string &file, const std::string &extention)
{
    if (file.length() > extention.length() && file[file.size() - 6] == '/') {
        return false;
    }
    return file.length() > extention.length() && (file.find_last_of('.') != std::string::npos &&
                                                  file.substr(file.find_last_of('.')) == extention);
}

}  // namespace utils
