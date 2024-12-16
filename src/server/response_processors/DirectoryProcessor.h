#ifndef WS_SERVER_RESPONSE_PROCESSORS_DIRECTORY_PROCESSOR_H
#define WS_SERVER_RESPONSE_PROCESSORS_DIRECTORY_PROCESSOR_H

#include <shared_ptr.h>
#include <unique_ptr.h>

#include "AResponseProcessor.h"
#include "Location.h"
#include "Request.h"
#include "response_processors/ErrorProcessor.h"

class DirectoryProcessor : public AResponseProcessor {
  private:
    enum DirEntryType {
        DE_FILE,
        DE_DIR
    };

  public:
    class DirEntry {
      public:
        DirEntry(const std::string& name, const std::string fpath, DirEntryType type,
                 std::time_t last_modified, size_t size)
            : name_(name), fpath_(fpath), type_(type), last_modified_(last_modified), size_(size)
        {}

        inline bool operator<(const DirEntry rhs) { return name_ < rhs.name_; }

        const std::string& name() const { return name_; }
        const std::string& fpath() const { return fpath_; }
        const DirEntryType& type() const { return type_; }
        const std::time_t& last_modified() const { return last_modified_; }
        size_t size() const { return size_; }

      private:
        std::string name_;
        std::string fpath_;
        DirEntryType type_;
        std::time_t last_modified_;
        size_t size_;
    };

  public:
    DirectoryProcessor(const Server& server,
                       utils::unique_ptr<http::IResponseCallback> response_rdy_cb,
                       const std::string& file_path, const std::string& root_dir,
                       const http::Request& rq);
    ~DirectoryProcessor(){};

  private:
    utils::unique_ptr<AResponseProcessor> err_response_processor_;
    const http::Request& rq_;
    bool ListDirectory_(const std::string& path, const std::string& location_root_dir);
    std::vector<DirEntry> entries_;

    std::pair<bool /*success*/, std::vector<DirEntry> /*dir_entries*/> GetDirEntries_(
        const char* directory);
};

#endif  // WS_SERVER_RESPONSE_PROCESSORS_DIRECTORY_PROCESSOR_H
