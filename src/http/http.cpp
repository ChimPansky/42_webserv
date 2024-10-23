#include "http.h"

namespace http {

Uri::Uri(const std::string& raw_uri) {
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

std::string Uri::ToStr() const {
    std::string str;
    if (!scheme_.empty()) {
        str += scheme_ + "://";
    }
    if (!host_.empty()) {
        str += host_;
    }
    if (port_ != 0) {
        str += ":" + /* todo: NumericToStr(port_) */ std::string("80");
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

void Uri::ParseStr_(const std::string& raw_uri) {
    host_ = raw_uri;
    //todo: go through raw_uri and parse
}

void Uri::Validate_() {
    state_ = URI_GOOD;
}

} // namespace http
