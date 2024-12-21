#include "file_utils.h"

#include <dirent.h>

#include <fstream>
#include <sstream>

#include "logger.h"
#include "rand.h"

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

bool IsRegularFile(const char *path)
{
    struct stat info;

    if (stat(path, &info) != 0) {
        return false;
    }
    return (info.st_mode & S_IFREG) != 0;
}

std::pair<bool /*success*/, std::string /*file_content*/> ReadFileToString(const char *filePath)
{
    if (!filePath[0]) {
        LOG(ERROR) << "Empty file path";
        return std::make_pair(false, "");
    }
    std::ifstream file(filePath);
    if (!file.is_open()) {
        LOG(ERROR) << "Could not open file: " << filePath;
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

std::pair<bool /*success*/, std::vector<utils::DirEntry> /*dir_entries*/> GetDirEntries(
    const char *directory)
{
    std::vector<utils::DirEntry> entries;
    DIR *dir = opendir(directory);
    if (dir == NULL) {
        LOG(ERROR) << "Unable to read directory: " << directory;
        return std::make_pair(false, entries);
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        std::string entry_path = std::string(directory) + "/" + entry->d_name;
        struct stat entry_stat;
        if (stat(entry_path.c_str(), &entry_stat) == 0) {
            time_t lastModified = entry_stat.st_mtime;
            size_t size = static_cast<size_t>(entry_stat.st_size);
            utils::DirEntryType type = (entry->d_type == DT_DIR ? utils::DE_DIR : utils::DE_FILE);
            std::string entry_name =
                std::string(entry->d_name) + (type == utils::DE_DIR ? "/" : "");

            entries.push_back(utils::DirEntry(entry_name, type, lastModified, size));
        } else {
            LOG(ERROR) << "Unable to read directory stats for file: " << entry_path;
        }
    }
    closedir(dir);
    return std::make_pair(true, entries);
}

std::pair<bool, std::string> CreateAndOpenTmpFileToStream(std::ofstream &fs)
{
    std::string tmp_name;
    do {
        tmp_name = TMP_DIR + GenerateRandomString(TMP_FILE_NAME_LEN);
    } while (DoesPathExist(tmp_name.c_str()));
    fs.open(tmp_name.c_str());
    return std::make_pair(fs.is_open(), tmp_name);
}

}  // namespace utils
