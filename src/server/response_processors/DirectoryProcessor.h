#ifndef WS_SERVER_RESPONSE_PROCESSORS_DIRECTORY_PROCESSOR_H
#define WS_SERVER_RESPONSE_PROCESSORS_DIRECTORY_PROCESSOR_H

#include <unique_ptr.h>

#include <fstream>

#include "AResponseProcessor.h"

class DirectoryProcessor : public AResponseProcessor {
  private:
    std::string GetContentType(const std::string& file);

  public:
    DirectoryProcessor(const std::string& file_path,
                  utils::unique_ptr<http::IResponseCallback> response_rdy_cb);
    ~DirectoryProcessor() {};

  private:
    utils::unique_ptr<GeneratedErrorResponseProcessor> err_response_processor_;
};

#endif  // WS_SERVER_RESPONSE_PROCESSORS_DIRECTORY_PROCESSOR_H
