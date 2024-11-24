#ifndef WS_SERVER_FILE_PROCESSOR_H
#define WS_SERVER_FILE_PROCESSOR_H

#include <ResponseCodes.h>
#include <fcntl.h>
#include <unique_ptr.h>
#include <unistd.h>

#include <fstream>

#include "AResponseProcessor.h"

class FileProcessor : public AResponseProcessor {
  private:
    std::vector<char> ReadFile_(std::ifstream& file)
    {
        return std::vector<char>(std::istreambuf_iterator<char>(file),
                                 std::istreambuf_iterator<char>());
    }
    std::string GetContentType(
        const std::string& file);  // return mime type based on extension (mb move to utils later)
  public:
    // change back to config
    FileProcessor(const std::string& file_path,
                  utils::unique_ptr<http::IResponseCallback> response_rdy_cb)
        : AResponseProcessor(response_rdy_cb),
          err_response_processor_(utils::unique_ptr<GeneratedErrorResponseProcessor>(NULL))
    {
        if (access(file_path.c_str(), F_OK) == -1) {
            err_response_processor_ = utils::unique_ptr<GeneratedErrorResponseProcessor>(
                new GeneratedErrorResponseProcessor(response_rdy_cb_, http::HTTP_NOT_FOUND));
            return;
        }
        std::ifstream file(file_path.c_str(), std::ios::binary);
        if (!file.is_open()) {
            err_response_processor_ = utils::unique_ptr<GeneratedErrorResponseProcessor>(
                new GeneratedErrorResponseProcessor(response_rdy_cb_,
                                                    http::HTTP_INTERNAL_SERVER_ERROR));
            return;
        }
        std::vector<char> body = ReadFile_(file);
        std::map<std::string, std::string> hdrs;
        hdrs["Content-Type"] = GetContentType(file_path);
        // hdrs["Connection"] = "Closed";
        hdrs["Content-Length"] = utils::NumericToString(body.size());
        response_rdy_cb_->Call(utils::unique_ptr<http::Response>(
            new http::Response(http::HTTP_OK, http::HTTP_1_1, hdrs, body)));
    }
    ~FileProcessor() {};

  private:
    utils::unique_ptr<GeneratedErrorResponseProcessor> err_response_processor_;
};

#endif  // WS_SERVER_FILE_PROCESSOR_H
