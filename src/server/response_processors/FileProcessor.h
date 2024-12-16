#ifndef WS_SERVER_RESPONSE_PROCESSORS_FILE_PROCESSOR_H
#define WS_SERVER_RESPONSE_PROCESSORS_FILE_PROCESSOR_H

#include <unique_ptr.h>

#include "AResponseProcessor.h"

class FileProcessor : public AResponseProcessor {
  private:
    std::string GetContentType(const std::string& file);

  public:
    FileProcessor(const Server& server, const std::string& file_path,
                  utils::unique_ptr<http::IResponseCallback> response_rdy_cb);
    ~FileProcessor(){};
};

#endif  // WS_SERVER_RESPONSE_PROCESSORS_FILE_PROCESSOR_H
