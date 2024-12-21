#ifndef WS_SERVER_RESPONSE_PROCESSORS_FILE_PROCESSOR_H
#define WS_SERVER_RESPONSE_PROCESSORS_FILE_PROCESSOR_H

#include <unique_ptr.h>

#include "AResponseProcessor.h"
#include "Location.h"

class FileProcessor : public AResponseProcessor {
  public:
    FileProcessor(const Server& server, const std::string& file_path,
                  utils::unique_ptr<http::IResponseCallback> response_rdy_cb,
                  const http::Request& rq, const Location& loc);
    ~FileProcessor() {};

  private:
    std::string GetContentType_(const std::string& file);
    void ProcessGet_(const std::string& file_path, const http::Request& rq, const Location& loc);
    void ProcessPost_(const std::string& file_path, const http::Request& rq, const Location& loc);
    void ProcessDelete_(const std::string& file_path, const Location& loc);
};

#endif  // WS_SERVER_RESPONSE_PROCESSORS_FILE_PROCESSOR_H
