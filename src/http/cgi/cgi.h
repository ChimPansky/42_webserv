#ifndef WS_CGI_CGI_H
#define WS_CGI_CGI_H

#include <unique_ptr.h>

#include <vector>

#include "../Request.h"
#include "../Response.h"

namespace cgi {

struct ScriptDetails {
    ScriptDetails(std::string location, std::string name, std::string extra_path)
        : location(location), name(name), extra_path(extra_path)
    {}
    std::string location;
    std::string name;
    std::string extra_path;
};

std::pair<bool, utils::unique_ptr<http::Response> > ParseCgiResponse(std::vector<char>& buf);

std::vector<std::string> GetEnv(const ScriptDetails& script, const http::Request& rq);

std::pair<bool, utils::unique_ptr<ScriptDetails> > GetScriptDetails(
    const std::string& path_from_url);

}  // namespace cgi

#endif  // WS_CGI_CGI_H
