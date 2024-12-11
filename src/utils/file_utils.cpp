#include "file_utils.h"

#include <fstream>
#include <sstream>
#include <logger.h>

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

std::pair<bool/*success*/, std::string/*file_content*/> ReadFileToString(const char* filePath) {
    if (!filePath[0]) {
        LOG(ERROR) << "utils::ReadFileToString: Empty file path";
        return std::make_pair<bool, std::string>(false, "");
    }
    std::ifstream file(filePath);
    if (!file.is_open()) {
        LOG(ERROR) << "utils::ReadFileToString: Could not open file: " << filePath;
        return std::make_pair<bool, std::string>(false, "");
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return std::make_pair<bool, std::string>(true, buffer.str());
}

}  // namespace utils