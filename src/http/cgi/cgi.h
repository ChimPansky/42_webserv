#ifndef WS_CGI_CGI_H
#define WS_CGI_CGI_H

#include <unique_ptr.h>

#include <vector>

#include "../Request.h"
#include "../Response.h"

namespace cgi {

utils::maybe<utils::unique_ptr<http::Response> > ParseCgiResponse(std::vector<char>& buf);

std::vector<std::string> GetEnv(const std::string& script_path, const http::Request& rq);

}  // namespace cgi

#endif  // WS_CGI_CGI_H
