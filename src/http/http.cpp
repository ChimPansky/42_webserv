#include "http.h"
#include <cstddef>
#include "utils/utils.h"

namespace http {

Uri::Uri(const std::string& raw_uri) : scheme_(""), host_(""), port_(0), path_(""), query_(""), fragment_("") {
    // parse raw_uri
    // scheme://host:port/path?query#fragment
    // scheme://host/path?query#fragment
    // scheme://host:port?query#fragment
    // scheme://host?query#fragment
    // scheme:path?query#fragment
    // scheme:/path?query#fragment
    // scheme:?query#fragment
    // scheme:?query
    // scheme://host:port
    // scheme://host
    // scheme:
    // scheme
    // scheme://
    // scheme:/
    // scheme://host:port/
    // scheme://host/
    // scheme://host:port/path
    // scheme://host/path
    // scheme://host:port/path?query
    // scheme://host/path?query
    // scheme://host:port?query
    // scheme://host?query
    // scheme:path?query
    // scheme:/path?query
    // scheme:?query
    // scheme://host:port#fragment
    // scheme://host#fragment
    // scheme://host:port/#fragment
    // scheme://host/#fragment
    // scheme://host:port/path#fragment
    // scheme://host/path#fragment
    // scheme://host:port/path?query#fragment
    // scheme://host/path?query#fragment
    // scheme://host:port?query#fragment
    // scheme://host?query#fragment
    // scheme:path?query#fragment
    // scheme:/path?query#fragment
    // scheme:?query#fragment
    // scheme:?query
    // scheme://host:port
    // scheme://host
    // scheme:
    // scheme
    // scheme://
    // scheme:/
    // scheme://host:port/
    // scheme://host/
    // scheme://host:port/path
    // scheme://host/path
    // scheme://host:port/path?query
    // scheme://host/path?query
    // scheme://host:port?query
    // scheme://host?query
    // scheme:path?query
    // scheme:/path?query
    // scheme:?query
    // scheme://host:port#fragment
    // scheme://host#fragment
    // scheme://host:port/#fragment
    // scheme://host/#fragment
    ParseStr_(raw_uri);
}

Uri::Uri(const std::string& scheme, const std::string& host, unsigned short port, const std::string& path, const std::string& query, const std::string& fragment)
    : scheme_(scheme), host_(host), port_(port), path_(path), query_(query), fragment_(fragment) {
    Validate_(); // todo: check if valid and set state if error
}

Uri::Uri(const Uri& other) {
    *this = other;
}

Uri& Uri::operator=(const Uri& other) {
    if (this != &other) {
        scheme_ = other.scheme_;
        host_ = other.host_;
        port_ = other.port_;
        path_ = other.path_;
        query_ = other.query_;
        fragment_ = other.fragment_;
    }
    return *this;
}

bool Uri::operator==(const Uri& other) const {
    return scheme_ == other.scheme_ && host_ == other.host_ && port_ == other.port_ && path_ == other.path_ && query_ == other.query_ && fragment_ == other.fragment_;
}

bool Uri::operator!=(const Uri& other) const {
    return !(*this == other);
}

std::string Uri::ToStr() const {
    std::string str;
    if (!scheme_.empty()) {
        str += scheme_ + "://";
    }
    if (!host_.empty()) {
        str += host_;
    }
    if (port_ != 0) {
        str += ":" + UnsignedShortToStr(port_);
    }
    if (!path_.empty()) {
        str += path_;
    }
    if (!query_.empty()) {
        str += "?" + query_;
    }
    if (!fragment_.empty()) {
        str += "#" + fragment_;
    }
    return str;
}

void Uri::Validate_() {
    state_ = URI_GOOD_BIT;
}

void Uri::ParseStr_(const std::string& raw_uri) {
    // host_ = raw_uri;
    // port_ = 0;
    //todo: go through raw_uri and parse
    size_t pos = 0;

    // scheme
    size_t scheme_end = raw_uri.find("://");
    if (scheme_end != std::string::npos) {
        scheme_ = raw_uri.substr(0, scheme_end);
        if (scheme_ != "http" && scheme_ != "https") {
            state_ = URI_BAD_SCHEME_BIT;
            return;
        }
        pos = scheme_end + 3;
    }

    // host & port
    size_t host_end = raw_uri.find_first_of(":/?#", pos);
    if (raw_uri[pos] != '/') {
        host_ = 
        //host_ = raw_uri.substr(pos, host_end - pos);
        if (host_end == std::string::npos) {
            return;
        }
        pos = host_end;
        if (!IsValidHost_(host_)) { // todo: check if valid host
            state_ = URI_BAD_HOST_BIT;
            return;
        }
        if (raw_uri[pos] == ':') {
            size_t port_end = raw_uri.find_first_of("/?#", pos + 1);
            std::string port_str = raw_uri.substr(pos + 1, port_end - pos - 1);
            std::pair<bool, unsigned short> port = utils::StrToNumericNoThrow<unsigned short>(port_str);
            if (port.first) {
                port_ = port.second;
            }
            else {
                state_ = URI_BAD_HOST_BIT;
                return;
            }
            if (port_end == std::string::npos) {
                return;
            }
        }
    }

    // path
    if (raw_uri[pos] == '/') {
        size_t path_end = raw_uri.find_first_of("?#", pos);
        path_ = raw_uri.substr(pos, path_end - pos);
        pos = path_end;
        if (!IsValidPath_(path_)) { // todo: check if valid path
            state_ = URI_BAD_PATH_BIT;
            return;
        }
        if (path_end == std::string::npos) {
            return;
        }
    }

    // query
    if (raw_uri[pos] == '?') {
        size_t query_end = raw_uri.find_first_of("#", pos);
        query_ = raw_uri.substr(pos + 1, query_end - pos - 1);
        pos = query_end;
        if (!IsValidQuery_(query_)) { // todo: check if valid query
            state_ = URI_BAD_QUERY_BIT;
            return;
        }
        if (query_end == std::string::npos) {
            return;
        }
    }

    // fragment
    if (raw_uri[pos] == '#') {
        size_t fragment_end = raw_uri.size();
        fragment_ = raw_uri.substr(pos + 1, fragment_end - pos - 1);
        if (!IsValidFragment_(fragment_)) {
            state_ = URI_BAD_FRAGMENT_BIT;
            return;
        }
    }
}

//helpers:

bool Uri::IsValidHostChar_(char c) const {
    return std::isalnum(c) || c == '.' || c == '-' || c == '_';
}

bool Uri::IsValidPathChar_(char c) const {
    return std::isalnum(c) || c == '.' || c == '-' || c == '_' || c == '/' || c == '?' || c == '#';
}

bool Uri::IsValidQueryOrFragmentChar_(char c) const {
    return std::isalnum(c) || c == '.' || c == '-' || c == '_' || c == '/' || c == '?' || c == '#' || c == '&';
}

bool Uri::IsValidHost_(const std::string& host) const {
    for (size_t i = 0; i < host.size(); ++i) {
        if (!IsValidHostChar_(host[i])) {   //todo: further checks
            return false;
        }
    }
    return true;
}

bool Uri::IsValidPath_(const std::string& path) const {
    for (size_t i = 0; i < path.size(); ++i) {
        if (!IsValidPathChar_(path[i])) {   //todo: further checks
            return false;
        }
    }
    return true;
}

bool Uri::IsValidQuery_(const std::string& query) const {
    for (size_t i = 0; i < query.size(); ++i) {
        if (!IsValidQueryOrFragmentChar_(query[i])) {   //todo: further checks
            return false;
        }
    }
    return true;
}

bool Uri::IsValidFragment_(const std::string &fragment) const {
    for (size_t i = 0; i < fragment.size(); ++i) {
        if (!IsValidQueryOrFragmentChar_(fragment[i])) {   //todo: further checks
            return false;
        }
    }
    return true;
}

} // namespace http
