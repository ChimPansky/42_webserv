#ifndef WS_SERVER_RESPONSE_PROCESSORS_DIRECTORY_PROCESSOR_H
#define WS_SERVER_RESPONSE_PROCESSORS_DIRECTORY_PROCESSOR_H

#include <shared_ptr.h>
#include <unique_ptr.h>

#include "AResponseProcessor.h"
#include "Location.h"
#include "Request.h"
#include "ResponseCodes.h"

class DirectoryProcessor : public AResponseProcessor {
  private:
    std::string GetContentType(const std::string& file);

    enum DirEntryType {
        DE_FILE,
        DE_FOLDER
    };
    class DirEntry {
      public:
        DirEntry(const std::string& name, const std::string fpath, DirEntryType type, bool visited)
            : name_(name), fpath_(fpath), type_(type), visited_(visited)
        {}

        std::string name() { return name_; }
        std::string fpath() { return fpath_; }
        DirEntryType type() { return type_; }
        bool visited() { return visited_; }

      private:
        std::string name_;
        std::string fpath_;
        DirEntryType type_;
        bool visited_;
        // change_date,..
    };

  public:
    DirectoryProcessor(const std::string& file_path,
                       utils::unique_ptr<http::IResponseCallback> response_rdy_cb,
                       const http::Request& rq, utils::shared_ptr<Location> loc);
    ~DirectoryProcessor(){};

  private:
    utils::unique_ptr<GeneratedErrorResponseProcessor> err_response_processor_;
    const http::Request& rq_;
    bool ListDirectory_(const std::string& path);
    std::vector<DirEntry> entries_;

    std::pair<bool /*success*/, std::vector<DirEntry> /*dir_entries*/> GetDirEntries_(
        const char* directory);
};

#endif  // WS_SERVER_RESPONSE_PROCESSORS_DIRECTORY_PROCESSOR_H
