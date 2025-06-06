#ifndef WS_SERVER_RESPONSE_PROCESSORS_DIRECTORY_PROCESSOR_H
#define WS_SERVER_RESPONSE_PROCESSORS_DIRECTORY_PROCESSOR_H

#include <shared_ptr.h>
#include <unique_ptr.h>

#include "AResponseProcessor.h"
#include "Request.h"
#include "file_utils.h"

class DirectoryProcessor : public AResponseProcessor {
  public:
    DirectoryProcessor(RequestDestination dest,
                       utils::unique_ptr<http::IResponseCallback> response_rdy_cb,
                       const http::Request& rq);
    ~DirectoryProcessor() {};

  private:
    bool ListDirectory_(const std::string& path);
    void GenerateAutoIndexPage_(std::ostringstream& body,
                                const std::vector<utils::DirEntry>& entries);

  private:
    static const char* kAutoIndexStyle();
};

#endif  // WS_SERVER_RESPONSE_PROCESSORS_DIRECTORY_PROCESSOR_H
