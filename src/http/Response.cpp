#include "Response.h"

#include <http.h>
#include <numeric_utils.h>

#include <string>

using namespace http;

Response::Response(ResponseCode code, http::Version version,
                   const std::map<std::string, std::string>& headers, const std::vector<char>& body)
    : code_(code), version_(version), headers_(headers), body_(body)
{}

Response::Response(ResponseCode code, http::Version version,
                   const std::map<std::string, std::string>& headers,
                   const std::string& body_file_path)
    : code_(code), version_(version), headers_(headers), body_file_path_(body_file_path)
{}

std::vector<char> Response::Dump() const
{
    std::string str_dump;
    str_dump += http::HttpVerToStr(version_);
    str_dump += " ";
    str_dump += utils::NumericToString(code_);
    str_dump += " ";
    str_dump += http::GetResponseCodeDescr(code_);
    str_dump += http::kCRLF();
    for (std::map<std::string, std::string>::const_iterator it = headers_.begin();
         it != headers_.end(); ++it) {
        str_dump += it->first;
        str_dump += ": ";
        str_dump += it->second;
        str_dump += http::kCRLF();
    }
    str_dump += http::kCRLF();

    std::vector<char> dump;
    std::copy(str_dump.begin(), str_dump.end(), std::back_inserter(dump));
    std::copy(body_.begin(), body_.end(), std::back_inserter(dump));
    return dump;
}

std::string Response::GetDebugString() const
{
    std::string str_dump;
    str_dump += http::HttpVerToStr(version_);
    str_dump += " ";
    str_dump += utils::NumericToString(code_);
    str_dump += " ";
    str_dump += http::GetResponseCodeDescr(code_);
    str_dump += http::kCRLF();
    for (std::map<std::string, std::string>::const_iterator it = headers_.begin();
         it != headers_.end(); ++it) {
        str_dump += it->first;
        str_dump += ": ";
        str_dump += it->second;
        str_dump += http::kCRLF();
    }
    str_dump += http::kCRLF();
    if (!body_.empty()) {
        str_dump += "has body of size: ";
        str_dump += utils::NumericToString(body_.size());
        str_dump += http::kCRLF();
    }
    if (body_file_path_) {
        str_dump += "body is in a file: ";
        str_dump += *body_file_path_;
        str_dump += http::kCRLF();
    }
    return str_dump;
}

const std::map<std::string, std::string>& Response::headers() const
{
    return headers_;
}

bool Response::AddHeader(const std::pair<std::string, std::string>& header)
{
    return headers_.insert(header).second;
}
