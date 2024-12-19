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

std::string UpdatePath(const std::string& loc, const std::string& matched_prefix,
                       const std::string& uri_path)
{
    std::string updated_path = loc.substr(1);
    std::string remaining_path = uri_path.substr(matched_prefix.length());
    if (remaining_path.empty()) {
        return updated_path;
    }
    if (*updated_path.rbegin() != '/' && *remaining_path.begin() != '/') {
        updated_path += "/";
    }
    updated_path += remaining_path;
    return updated_path;
}

}  // namespace utils
