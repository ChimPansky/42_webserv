#include "utils.h"

namespace utils {

const char* GetTypeByExt(const std::string& ext)
{
    if (ext == ".html")
        return "text/html";
    else if (ext == ".css")
        return "text/css";
    else if (ext == ".js")
        return "application/javascript";
    else if (ext == ".txt")
        return "text/plain";
    else if (ext == ".png")
        return "image/png";
    else if (ext == ".jpg" || ext == ".jpeg")
        return "image/jpeg";
    else if (ext == ".gif")
        return "image/gif";
    else if (ext == ".ico")
        return "image/x-icon";
    else if (ext == ".pdf")
        return "application/pdf";
    else if (ext == ".json")
        return "application/json";
    else
        return kDefaultContentType();
}

std::string GetInterpreterByExt_(const std::string& filename)
{
    if (filename.find_last_of(".") == std::string::npos) {
        return std::string();
    }
    std::string ext = filename.substr(filename.find_last_of("."));
    if (ext == ".py") {
        return "/usr/bin/python3";
    } else if (ext == ".php") {
        return "/usr/bin/php";
    } else if (ext == ".pl") {
        return "/usr/bin/perl";
    } else if (ext == ".sh") {
        return "/bin/sh";
    }
    return std::string();
}

std::string UpdatePath(const std::string& loc, const std::string& matched_prefix,
                       const std::string& uri_path)
{
    std::string remaining_path = uri_path.substr(matched_prefix.length());
    if (remaining_path[0] != '/') {
        remaining_path = "/" + remaining_path;
    }
    return loc.substr(1) + remaining_path;
}

}  // namespace utils
