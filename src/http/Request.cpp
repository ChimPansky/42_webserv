#include "Request.h"

#include <logger.h>
#include <str_utils.h>

#include <sstream>

#include "RqTarget.h"
#include "http.h"

namespace http {

Request::Request() : status(HTTP_OK), method(HTTP_NO_METHOD), version(HTTP_NO_VERSION)
{}

BodyStorage::BodyStorage() : storage_type(BST_IN_TMP_FOLDER), transfer_complete(false)
{}

BodyStorage::~BodyStorage()
{
    if ((storage_type == BST_IN_TMP_FOLDER || !transfer_complete) &&
        utils::DoesPathExist(path.c_str())) {
        remove(path.c_str());
    }
}

bool Request::has_body() const
{
    return body.path[0] != '\0';
}

utils::maybe<std::string> Request::GetHeaderVal(const std::string& key) const
{
    std::map<std::string, std::string>::const_iterator it = headers.find(utils::ToLowerCase(key));
    if (it != headers.end()) {
        return it->second;
    }
    return utils::maybe_not();
}

std::string Request::GetDebugString() const
{
    std::ostringstream ret;
    ret << "---Request---"
        << "\n\tStatus: " << (status == HTTP_OK ? "OK " : "BAD ") << status
        << "\n\tMethod: " << method << "\n\tMethod: " << HttpMethodToStr(method)
        << "\n\tRequest-Target: " << rqTarget.ToStr() << "\n\tVersion: " << HttpVerToStr(version)
        << "\n\t~Headers~";

    for (std::map<std::string, std::string>::const_iterator it = headers.begin();
         it != headers.end(); ++it) {
        ret << "\n\t" << it->first << ": " << it->second;
    }

    ret << "\n\tHas body: " << (has_body() ? "Yes" : "No");
    ret << "\n\tBody storage type: "
        << (body.storage_type == BST_IN_TMP_FOLDER ? "In tmp folder" : "On server");
    ret << "\n\tBodypath: " << body.path;

    return ret.str();
}

}  // namespace http
