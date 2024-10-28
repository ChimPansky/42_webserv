#include "Response.h"

using namespace http;

#include <string>

#define HTTP_RESPONSE \
    "HTTP/1.1 200 OK\r\n\
Date: Mon, 27 Jul 2009 12:28:53 GMT\n\r\
Server: ft_webserv\n\r\
Last-Modified: Wed, 22 Jul 2009 19:15:56 GMT\n\r\
Content-Length: 88\n\r\
Content-Type: text/html\n\r\
Connection: Closed\n\r\
\n\r\
<html>\n\r\
<body>\n\r\
<h1>Hello, World!</h1>\n\r\
</body>\n\r\
</html>\n\r"

std::vector<char> Response::Dump() const {
    (void) code_;
    std::vector<char> buf;
    std::string str(HTTP_RESPONSE);
    std::copy(str.begin(), str.end(), std::back_inserter(buf));
    return buf;
}
