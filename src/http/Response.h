#ifndef WS_HTTP_RESPONSE_H
#define WS_HTTP_RESPONSE_H

#include <ResponseCodes.h>
#include <numeric_utils.h>
#include <time_utils.h>
#include <unique_ptr.h>

#include <map>
#include <string>
#include <vector>

#include "http.h"

namespace http {

class Response {
  public:
    Response(ResponseCode code, http::Version version,
             const std::map<std::string, std::string>& headers, const std::vector<char>& body);
    Response(ResponseCode code, http::Version version,
             const std::map<std::string, std::string>& headers, const std::string& body_file_path);

  public:
    std::vector<char> Dump() const;
    std::string GetDebugString() const;
    const std::map<std::string, std::string>& headers() const;
    bool AddHeader(const std::pair<std::string, std::string>& header);
    const utils::maybe<std::string>& body_file_path() { return body_file_path_; }

  private:
    ResponseCode code_;
    http::Version version_;
    std::map<std::string, std::string> headers_;
    std::vector<char> body_;
    utils::maybe<std::string> body_file_path_;
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
Connection: Close\n\r\
\n\r\
<html>\n\r\
<body>\n\r\
<h1>Hello, World!</h1>\n\r\
</body>\n\r\
</html>\n\r"
*/
inline utils::unique_ptr<http::Response> GetSimpleValidResponse()
{
    std::string txt_body =
        "<html>\n\r"
        "<body>\n\r"
        "<h1>Hello, World!</h1>\n\r"
        "</body>\n\r"
        "</html>\n\r";

    std::map<std::string, std::string> hdrs;
    hdrs["Last-Modified"] = "Wed, 22 Jul 2009 19:15:56 GMT";
    hdrs["Content-Length"] = utils::NumericToString(txt_body.size());
    hdrs["Content-Type"] = "text/html";
    hdrs["Connection"] = "Close";

    std::vector<char> body;
    std::copy(txt_body.begin(), txt_body.end(), std::back_inserter(body));
    return utils::unique_ptr<http::Response>(
        new http::Response(http::HTTP_OK, HTTP_1_1, hdrs, body));
}

}  // namespace http

#endif  // WS_HTTP_RESPONSE_H
