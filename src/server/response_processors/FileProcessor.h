#ifndef WS_SERVER_RESPONSE_PROCESSORS_FILE_PROCESSOR_H
#define WS_SERVER_RESPONSE_PROCESSORS_FILE_PROCESSOR_H

#include <unique_ptr.h>

#include <fstream>

#include "AResponseProcessor.h"
#include "DirectoryProcessor.h"

class FileProcessor : public AResponseProcessor {
  private:
    std::string GetContentType(const std::string& file);

  public:
    FileProcessor(const std::string& file_path,
                  utils::unique_ptr<http::IResponseCallback> response_rdy_cb);
    ~FileProcessor() {};

  private:
    utils::unique_ptr<GeneratedErrorResponseProcessor> err_response_processor_;
    utils::unique_ptr<DirectoryProcessor> directory_processor_;
};

#endif  // WS_SERVER_RESPONSE_PROCESSORS_FILE_PROCESSOR_H
