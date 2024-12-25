#ifndef WS_SERVER_RESPONSE_PROCESSORS_FILE_PROCESSOR_H
#define WS_SERVER_RESPONSE_PROCESSORS_FILE_PROCESSOR_H

#include <unique_ptr.h>

#include "AResponseProcessor.h"
#include "Location.h"

class FileProcessor : public AResponseProcessor {
  public:
    FileProcessor(RequestDestination dest,
                  utils::unique_ptr<http::IResponseCallback> response_rdy_cb,
                  const http::Request& rq);
    ~FileProcessor() {};

  private:
    std::string GetContentType_(const std::string& file);
    void ProcessGet_(const http::Request& rq);
    void ProcessPost_();
    void ProcessDelete_();
};

#endif  // WS_SERVER_RESPONSE_PROCESSORS_FILE_PROCESSOR_H
