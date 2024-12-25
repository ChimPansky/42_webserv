#include "file_utils.h"

#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include <fstream>
#include <sstream>

#include "logger.h"
#include "numeric_utils.h"
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

bool TryChangeDir(const char *path)
{
    return chdir(path) != -1;
}

utils::maybe<std::string> ReadFileToString(const char *filePath)
{
    if (!filePath[0]) {
        LOG(ERROR) << "Empty file path";
        return utils::maybe_not();
    }
    std::ifstream file(filePath);
    if (!file.is_open()) {
        LOG(ERROR) << "Could not open file: " << filePath;
        return utils::maybe_not();
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

bool CheckFileExtension(const std::string &file, const std::string &extention)
{
    if (file.length() > extention.length() && file[file.size() - 6] == '/') {
        return false;
    }
    return file.length() > extention.length() && (file.find_last_of('.') != std::string::npos &&
                                                  file.substr(file.find_last_of('.')) == extention);
}

utils::maybe<std::vector<utils::DirEntry> > GetDirEntries(const char *directory)
{
    std::vector<utils::DirEntry> entries;
    DIR *dir = opendir(directory);
    if (dir == NULL) {
        LOG(ERROR) << "Unable to read directory: " << directory;
        return utils::maybe_not();
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
    return entries;
}

// this shit is evil but works (but not with valgrind)
bool CloseProcessFdsButStd()
{
    // cannot close right away cuz fd_dir is also there
    std::vector<int> fds_to_close;

    DIR *fd_dir = opendir("/proc/self/fd");
    if (!fd_dir) {
        LOG(ERROR) << "Unable to read directory: /proc/self/fd";
        return false;
    }
    struct dirent *entry;
    while ((entry = readdir(fd_dir)) != NULL) {
        maybe<int> fd = StrToNumericNoThrow<int>(entry->d_name);
        if (!fd || *fd == STDIN_FILENO || *fd == STDOUT_FILENO || *fd == STDERR_FILENO) {
            continue;
        }
        fds_to_close.push_back(*fd);
    }
    closedir(fd_dir);
    for (std::vector<int>::iterator it = fds_to_close.begin(); it != fds_to_close.end(); ++it) {
        close(*it);
    }
    return true;
}

utils::maybe<std::string> CreateAndOpenTmpFileToStream(std::ofstream &fs)
{
    std::string tmp_name;
    do {
        tmp_name = TMP_DIR + GenerateRandomString(TMP_FILE_NAME_LEN);
    } while (DoesPathExist(tmp_name.c_str()));
    fs.open(tmp_name.c_str());
    if (!fs.is_open()) {
        LOG(DEBUG) << "Failed to open temporary file: " << tmp_name;
        return maybe_not();
    }
    return tmp_name;
}

}  // namespace utils
