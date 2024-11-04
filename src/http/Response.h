#ifndef WS_HTTP_RESPONSE_H
#define WS_HTTP_RESPONSE_H

#include <vector>
#include <string>
#include <map>
#include <unique_ptr.h>
#include <stdexcept>
#include "http.h"
#include <numeric_utils.h>
#include <time_utils.h>

namespace http {

enum ResponseCode {
    OK                      = 200,
    BAD_REQUEST             = 400,
    NOT_FOUND               = 404,
    INTERNAL_SERVER_ERROR   = 500,
};

inline const char* ResponseCodeHint(ResponseCode rs_code) {
  switch (rs_code) {
    case OK:                          return "Ok";
    case BAD_REQUEST:                 return "Bad Request";
    case NOT_FOUND:                   return "Not Found";
    case INTERNAL_SERVER_ERROR:       return "Internal Server Error";
  }
  throw std::logic_error("unlnown http code");
}

class Response {
  public:
    // class ResponseBuilder {
    //   public:
    //     ResponseBuilder(utils::unique_ptr<Response> rs) : rs_(rs), complete_(true) {};
    //     ResponseBuilder(utils::unique_ptr<IResponseProcessor> response_processor) : response_processor_(response_processor), complete_(false) {
    //       if (response_processor_->IsDone()) {
    //         rs_ = response_processor_.GetResponse();
    //         Finalize();
    //       }
    //     };
    //     const utils::unique_ptr<IResponseProcessor>& response_processor() const {return response_processor_;}
    //     bool Complete() const {return complete_;}
    //     int ProcessorFd() const {return response_processor_.fd();}
    //     void AddToBody(size_t sz, const char* buf) {
    //         rs_->body_.reserve(rs_->body_.size() + sz);
    //         std::copy(buf, buf + sz, std::back_inserter(rs_->body_));
    //     }
    //     void Finalize() {
    //         complete_ = true;
    //     }
    //     utils::unique_ptr<Response> GetResponse() {
    //         if (complete_) {
    //             return rs_;
    //         } else {
    //             return  utils::unique_ptr<Response>();
    //         }
    //     }
    //   private:
    //     utils::unique_ptr<Response> rs_;
    //     utils::unique_ptr<IResponseProcessor> response_processor_;
    //     bool complete_;
    // };
    Response(ResponseCode code, http::Version version, const std::map<std::string, std::string>& headers, const std::vector<char>& body);
  public:
    std::vector<char> Dump() const;
    std::string DumpToStr() const;
  private:
    ResponseCode code_;
    http::Version version_;
    std::map<std::string, std::string> headers_;
    std::vector<char> body_;
};

class IResponseCallback {
  public:
    virtual void Call(utils::unique_ptr<http::Response> rs) = 0;
    virtual ~IResponseCallback() {};
};

/*
"HTTP/1.1 200 OK"\r\n"\
Date: Mon, 27 Jul 2009 12:28:53 GMT\n\r\
Server: \n\r\
Last-Modified: \n\r\
Content-Length: 88\n\r\
Content-Type: text/html\n\r\
Connection: Closed\n\r\
\n\r\
<html>\n\r\
<body>\n\r\
<h1>Hello, World!</h1>\n\r\
</body>\n\r\
</html>\n\r"
*/
inline utils::unique_ptr<http::Response> GetSimpleValidResponse() {
    std::string txt_body =
      "<html>\n\r"
      "<body>\n\r"
      "<h1>Hello, World!</h1>\n\r"
      "</body>\n\r"
      "</html>\n\r";

    std::map<std::string, std::string> hdrs;
    hdrs["Date"] = utils::GetFormatedTime();
    hdrs["Server"] = "ft_webserv";
    hdrs["Last-Modified"] = "Wed, 22 Jul 2009 19:15:56 GMT";
    hdrs["Content-Length"] = utils::NumericToString(txt_body.size());
    hdrs["Content-Type"] = "text/html";
    hdrs["Connection"] = "Closed";

    std::vector<char> body;
    std::copy(txt_body.begin(), txt_body.end(), std::back_inserter(body));
    return utils::unique_ptr<http::Response>(new http::Response(http::OK, HTTP_1_1, hdrs, body));
}

}  // namespace http

#endif  // WS_HTTP_RESPONSE_H
