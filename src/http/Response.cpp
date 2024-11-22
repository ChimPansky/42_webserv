#include <numeric_utils.h>
#include "Response.h"
#include <string>
#include <http.h>

using namespace http;

Response::Response(ResponseCode code, http::Version version, const std::map<std::string, std::string>& headers, const std::vector<char>& body) : code_(code), version_(version), headers_(headers), body_(body) {}

std::vector<char> Response::Dump() const {
    std::string str_dump;
    str_dump += http::HttpVerToStr(version_);
    str_dump += " ";
    str_dump += utils::NumericToString(code_);
    str_dump += " ";
    str_dump += http::GetResponseCodeDescr(code_);
    str_dump += http::LineSep();
    for (std::map<std::string, std::string>::const_iterator it = headers_.begin(); it != headers_.end(); ++it) {
        str_dump += it->first;
        str_dump += ": ";
        str_dump += it->second;
        str_dump += http::LineSep();
    }
    str_dump += http::LineSep();  // if no body?
    std::vector<char> dump;
    std::copy(str_dump.begin(), str_dump.end(), std::back_inserter(dump));
    std::copy(body_.begin(), body_.end(), std::back_inserter(dump));
    return dump;
}

std::string Response::DumpToStr() const {
    std::string str_dump;
    str_dump += http::HttpVerToStr(version_);
    str_dump += " ";
    str_dump += utils::NumericToString(code_);
    str_dump += " ";
    str_dump += http::GetResponseCodeDescr(code_);
    str_dump += http::LineSep();
    for (std::map<std::string, std::string>::const_iterator it = headers_.begin(); it != headers_.end(); ++it) {
        str_dump += it->first;
        str_dump += ": ";
        str_dump += it->second;
        str_dump += http::LineSep();
    }
    str_dump += http::LineSep();  // if no body?
    std::copy(body_.begin(), body_.end(), std::back_inserter(str_dump));
    return str_dump;
}

const std::map<std::string, std::string>& Response::headers() const {
    return headers_;
}

void  Response::set_headers(const std::map<std::string, std::string>& hdrs)
{
    headers_ = hdrs;
}
