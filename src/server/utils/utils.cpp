#include "utils.h"

namespace utils {

std::map<const char*, const char*> GetMimeTypes()
{
    static std::map<const char*, const char*> mime_types;

    if (mime_types.empty()) {  // to initialize it only once
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
    }

    return mime_types;
}

}  // namespace utils
