#ifndef WS_UTILS_FILE_UTILS_H
#define WS_UTILS_FILE_UTILS_H

#include <fstream>
#include <string>
#include <vector>

#include "maybe.h"
#include "time_utils.h"

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

utils::maybe<std::string> ReadFileToString(const char *filePath);

inline utils::maybe<std::string> ReadFileToString(const std::string &filePath)
{
    return ReadFileToString(filePath.c_str());
}


bool CheckFileExtension(const std::string &file, const std::string &extention);

maybe<std::string /*filename*/> CreateAndOpenTmpFileToStream(std::ofstream &fs);

enum DirEntryType {
    DE_FILE,
    DE_DIR
};

class DirEntry {
  public:
    DirEntry(const std::string &name, DirEntryType type, UnixTimestampS last_modified, size_t size)
        : name_(name), type_(type), last_modified_(last_modified), size_(size)
    {}

    const std::string &name() const { return name_; }
    DirEntryType type() const { return type_; }
    UnixTimestampS last_modified() const { return last_modified_; }
    size_t size() const { return size_; }

    inline bool operator<(const DirEntry rhs) { return name_ < rhs.name_; }

  private:
    std::string name_;
    DirEntryType type_;
    UnixTimestampS last_modified_;
    size_t size_;
};

utils::maybe<std::vector<DirEntry> > GetDirEntries(const char *directory);

}  // namespace utils

#endif  // WS_UTILS_FILE_UTILS_H
