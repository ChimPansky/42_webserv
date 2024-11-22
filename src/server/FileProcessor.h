#ifndef WS_SERVER_FILE_PROCESSOR_H
#define WS_SERVER_FILE_PROCESSOR_H

#include <fcntl.h>
#include <unistd.h>

#include "AResponseProcessor.h"
#include <ResponseCodes.h>
#include <unique_ptr.h>

class FileProcessor : public AResponseProcessor {
  private:
    std::string ReadFile_();
    std::string GetContentType(const std::string& file); // return mime type based on extension (mb move to utils later)
  public:
    // change back to config
    FileProcessor(const std::string& file_path,
                  utils::unique_ptr<http::IResponseCallback> response_rdy_cb)
        : AResponseProcessor(response_rdy_cb), err_response_processor_(utils::unique_ptr<GeneratedErrorResponseProcessor>(NULL))
    {
        if (access(file_path.c_str(), F_OK) == -1) {
            // return 404
            err_response_processor_ = utils::unique_ptr<GeneratedErrorResponseProcessor>(new GeneratedErrorResponseProcessor(response_rdy_cb_, http::HTTP_NOT_FOUND));
            return ;
        }
        fd_ = open(file_path.c_str(), O_RDONLY);
        if (fd_ == -1) {
            // return 50x (for now 500)
          err_response_processor_ = utils::unique_ptr<GeneratedErrorResponseProcessor>(new GeneratedErrorResponseProcessor(response_rdy_cb_, http::HTTP_NOT_FOUND));
          return ;
        }
        std::string body_str = ReadFile_();
        std::vector<char> body;
        body.reserve(body_str.size());
        std::copy(body_str.begin(), body_str.end(), std::back_inserter(body));
        std::map<std::string, std::string> hdrs;
        hdrs["Content-Type"] = GetContentType(file_path);
        // hdrs["Connection"] = "Closed";
        hdrs["Content-Length"] = utils::NumericToString(body.size());
        response_rdy_cb_->Call(utils::unique_ptr<http::Response>(
            new http::Response(http::HTTP_OK, http::HTTP_1_1, hdrs, body)));
    }
    ~FileProcessor() { close(fd_); };

  private:
    int fd_;
    utils::unique_ptr<GeneratedErrorResponseProcessor>  err_response_processor_;
};

#endif  // WS_SERVER_FILE_PROCESSOR_H
