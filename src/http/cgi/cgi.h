#ifndef WS_CGI_CGI_H
#define WS_CGI_CGI_H

#include <unique_ptr.h>

#include <vector>

#include "../Request.h"
#include "../Response.h"

namespace cgi {

struct ScriptDetails {
    ScriptDetails(const std::string& location, const std::string& name,
                  const std::string& extra_path, const std::string& full_path)
        : location(location), name(name), extra_path(extra_path), full_path(full_path)
    {}
    std::string location;
    std::string name;
    std::string extra_path;
    std::string full_path;
};

utils::maybe<utils::unique_ptr<http::Response> > ParseCgiResponse(std::vector<char>& buf);

std::vector<std::string> GetEnv(const ScriptDetails& script, const http::Request& rq);

std::pair<bool, utils::unique_ptr<ScriptDetails> > GetScriptDetails(
    const std::string& path_from_url, const std::string& alias_dir);

}  // namespace cgi

#endif  // WS_CGI_CGI_H
