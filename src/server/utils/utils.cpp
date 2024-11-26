#include "utils.h"

namespace utils {

std::map<std::string, std::string> GetMimeTypes()
{
    std::map<std::string, std::string> mime_types;

    mime_types[".html"] = "text/html";
    mime_types[".css"] = "text/css";
    mime_types[".js"] = "application/javascript";
    mime_types[".txt"] = "text/plain";
    mime_types[".png"] = "image/png";
    mime_types[".jpg"] = "image/jpeg";
    mime_types[".jpeg"] = "image/jpeg";
    mime_types[".gif"] = "image/gif";
    mime_types[".ico"] = "image/x-icon";
    mime_types[".pdf"] = "application/pdf";
    mime_types[".json"] = "application/json";

    return mime_types;
}

}  // namespace utils
