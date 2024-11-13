#ifndef WS_SERVER_IPROCESSOR_H
#define WS_SERVER_IPROCESSOR_H

#include <Request.h>
#include <Response.h>
#include <unique_ptr.h>
#include <time_utils.h>

class ClientSession;

class AResponseProcessor {
  protected:
    AResponseProcessor(utils::unique_ptr<http::IResponseCallback> response_rdy_cb) : response_rdy_cb_(response_rdy_cb) {};
  public:
    virtual ~AResponseProcessor() {};
  protected:
    utils::unique_ptr<http::IResponseCallback> response_rdy_cb_;
};

class HelloWorldResponseProcessor : public AResponseProcessor {
  public:
    HelloWorldResponseProcessor(utils::unique_ptr<http::IResponseCallback> response_rdy_cb) : AResponseProcessor(response_rdy_cb) {
      response_rdy_cb_->Call(http::GetSimpleValidResponse());
    }
};

class GeneratedErrorResponseProcessor : public AResponseProcessor {
  public:
    GeneratedErrorResponseProcessor(utils::unique_ptr<http::IResponseCallback> response_rdy_cb, http::ResponseCode code) : AResponseProcessor(response_rdy_cb) {
      std::string body_str = GenerateErrorPage_(code);
      std::vector<char> body;
      body.reserve(body_str.size());
      std::copy(body_str.begin(), body_str.end(), std::back_inserter(body));
      std::map<std::string, std::string> hdrs;
      hdrs["Server"] = "ft_webserv";
      hdrs["Date"] = utils::GetFormatedTime();
      hdrs["Content-Type"] = "text/html";
      // hdrs["Connection"] = "Closed";
      hdrs["Content-Length"] = utils::NumericToString(body.size());
      response_rdy_cb_->Call(utils::unique_ptr<http::Response>(new http::Response(code, http::HTTP_1_1, hdrs, body)));
    }
  private:
    std::string GenerateErrorPage_(http::ResponseCode code) {
        std::stringstream ss;
        ss << "<!DOCTYPE html>\n"
            << "<html lang=\"en\">"
            << "<head>"
            << "<meta charset=\"UTF-8\">"
            << "<title>" << code << " " << http::GetResponseCodeDescr(code) << "</title>"
            << "</head>"
            << "<body style=\"font-family: Arial, sans-serif; text-align: center; color: black; background-color: white; padding: 20px;\">"
            << "<h1 style=\"font-size: 48px; margin: 0;\">" << code << "</h1>"
            << "<p style=\"font-size: 16px; margin: 10px 0;\">" << http::GetResponseCodeDescr(code) << "</p>"
            << "</body>"
            << "</html>";
        return ss.str();
    }
};

// class ReadFileResponseProcessor : public AResponseProcessor {
//   ReadFileResponseProcessor(utils::unique_ptr<http::IResponseCallback> response_rdy_cb) : AResponseProcessor(response_rdy_cb) {
//     std::vector<char> file_content;
//     std::string a = "asd";
//     std::copy(a.begin(), a.end(), std::back_inserter(body));
//     response_rdy_cb->Call(Response);
//   }
// }

// #include <fcntl.h>
// #include <unistd.h>
// class FileProcessor : public IResponseProcessor {
//   public:
//     // change back to config
//     FileProcessor(const std::string& file_path) {
//       fd_ = open(file_path.c_str(), O_RDONLY);
//     }
//     ~FileProcessor() {close(fd_);};
//   private:
//     int fd_;
// };


#endif  // WS_SERVER_IPROCESSOR_H
