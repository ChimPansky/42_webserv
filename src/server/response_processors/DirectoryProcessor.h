#ifndef WS_SERVER_RESPONSE_PROCESSORS_DIRECTORY_PROCESSOR_H
#define WS_SERVER_RESPONSE_PROCESSORS_DIRECTORY_PROCESSOR_H

#include <shared_ptr.h>
#include <unique_ptr.h>

#include "AResponseProcessor.h"
#include "Location.h"
#include "Request.h"

class DirectoryProcessor : public AResponseProcessor {
  private:
    std::string GetContentType(const std::string& file);

    enum DirEntryType {
        DE_FILE,
        DE_DIR
    };
    class DirEntry {
      public:
        DirEntry(const std::string& name, const std::string fpath, DirEntryType type)
            : name_(name), fpath_(fpath), type_(type)
        {}

        std::string name() { return name_; }
        std::string fpath() { return fpath_; }
        DirEntryType type() { return type_; }

      private:
        std::string name_;
        std::string fpath_;
        DirEntryType type_;
        // todo: maybe implement bool visited_;
        // todo: last_modified,..
    };

  public:
    DirectoryProcessor(utils::unique_ptr<http::IResponseCallback> response_rdy_cb,
                       const std::string& file_path, const http::Request& rq,
                       utils::shared_ptr<Location> loc);
    ~DirectoryProcessor(){};

  private:
    utils::unique_ptr<GeneratedErrorResponseProcessor> err_response_processor_;
    const http::Request& rq_;
    bool ListDirectory_(const std::string& path, const std::string& location_root_dir);
    std::vector<DirEntry> entries_;

    std::pair<bool /*success*/, std::vector<DirEntry> /*dir_entries*/> GetDirEntries_(
        const char* directory);
    std::string RemoveRootFromPath(const std::string& path, const std::string& root);
};

#endif  // WS_SERVER_RESPONSE_PROCESSORS_DIRECTORY_PROCESSOR_H
