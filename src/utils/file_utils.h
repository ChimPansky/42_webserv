#ifndef WS_UTILS_FILE_UTILS_H
#define WS_UTILS_FILE_UTILS_H

#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cstdio>
#include <string>
#include <vector>

#include "rand.h"

#define TMP_FILE_NAME_LEN 20
// todo move to config?
#define TMP_DIR "./tmp/"

namespace utils {

const static int kMaxTempFileName = TMP_MAX;

bool DoesPathExist(const char *path);

bool IsReadable(const char *path);

bool IsExecutable(const char *path);

bool IsDirectory(const char *path);

bool IsRegularFile(const char *path);

std::pair<bool /*success*/, std::string /*file_content*/> ReadFileToString(const char *filePath);

inline std::pair<bool /*success*/, std::string /*file_content*/> ReadFileToString(
    const std::string &filePath)
{
    return ReadFileToString(filePath.c_str());
}


bool CheckFileExtension(const std::string &file, const std::string &extention);

template <class FileStream>
std::pair<bool, std::string> CreateAndOpenTmpFileToStream(FileStream &fs)
{
    std::string tmp_name;
    do {
        tmp_name = TMP_DIR + GenerateRandomString(TMP_FILE_NAME_LEN);
    } while (access(tmp_name.c_str(), F_OK) == 0);
    // Create and use the file
    fs.open(tmp_name.c_str());
    return std::make_pair(fs.is_open(), tmp_name);
}

enum DirEntryType {
    DE_FILE,
    DE_DIR
};

class DirEntry {
  public:
    DirEntry(const std::string &name, DirEntryType type, time_t last_modified, size_t size)
        : name_(name), type_(type), last_modified_(last_modified), size_(size)
    {}

    const std::string &name() const { return name_; }
    const DirEntryType &type() const { return type_; }
    const time_t &last_modified() const { return last_modified_; }
    size_t size() const { return size_; }

    inline bool operator<(const DirEntry rhs) { return name_ < rhs.name_; }

  private:
    std::string name_;
    DirEntryType type_;
    time_t last_modified_;
    size_t size_;
};

std::pair<bool /*success*/, std::vector<DirEntry> /*dir_entries*/> GetDirEntries(
    const char *directory);

}  // namespace utils

#endif  // WS_UTILS_FILE_UTILS_H
