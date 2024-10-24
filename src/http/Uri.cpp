#include "Uri.h"
#include "http.h"
#include "utils/utils.h"

namespace http {

Uri::Uri(const std::string& raw_uri) : state_(PS_SCHEME), status_(URI_GOOD_BIT), scheme_(""), host_(""), port_(0), path_(""), query_(""), fragment_("") {
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
    ParseRawUri_(raw_uri);
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
    status_ = URI_GOOD_BIT;
}

void Uri::ParseRawUri_(const std::string& raw_uri) {
    // host_ = raw_uri;
    // port_ = 0;
    //todo: go through raw_uri and parse
    raw_uri_pos_ = 0;
    while (state_ != PS_END && status_ == URI_GOOD_BIT) {
        switch (state_) {
            case PS_SCHEME: ParseScheme_(raw_uri); break;
            case PS_HOST: ParseHost_(raw_uri); break;
            case PS_PORT: ParsePort_(raw_uri); break;
            case PS_PATH: ParsePath_(raw_uri); break;
            case PS_QUERY: ParseQuery_(raw_uri); break;
            case PS_FRAGMENT: ParseFragment_(raw_uri); break;
            case PS_END: break;
        }
    }
}

void Uri::ParseScheme_(const std::string& raw_uri) {
   // scheme
    size_t scheme_end = raw_uri.find("://");
    if (scheme_end == std::string::npos) {
        state_ = PS_PATH;
        return;
    }
    scheme_ = raw_uri.substr(0, scheme_end);
    if (scheme_ != "http" && scheme_ != "https") {
        status_ = URI_BAD_SCHEME_BIT;
        return;
    }
    raw_uri_pos_ = scheme_end + 3;
    state_ = PS_HOST;
}

void Uri::ParseHost_(const std::string& raw_uri) {
    if (EndOfRawUri_(raw_uri)) {
        status_ = URI_BAD_HOST_BIT;
        return;
    }
    size_t host_end = raw_uri.find_first_of(":/", raw_uri_pos_);
    if (host_end == std::string::npos) {
        status_ = URI_BAD_PATH_BIT;
        return;
    }
    host_ = raw_uri.substr(raw_uri_pos_, host_end - raw_uri_pos_);
    raw_uri_pos_ = host_end;
    if (!IsValidHost_(host_)) {
        status_ = URI_BAD_HOST_BIT;
        return;
    }
    if (raw_uri[host_end] == ':') {
        state_ = PS_PORT;
        return;
    }
    if (raw_uri[host_end] == '/') {
        state_ = PS_PATH;
        return;
    }
    state_ = PS_END; // should never reach here
}

void Uri::ParsePort_(const std::string& raw_uri) {
    if (EndOfRawUri_(raw_uri) || raw_uri[raw_uri_pos_] != ':') {
        status_ = URI_BAD_PORT_BIT;
        return;
    }
    size_t port_end = raw_uri.find_first_of("/", raw_uri_pos_);
    if (port_end == std::string::npos) {
        status_ = URI_BAD_PATH_BIT;
        return;
    }
    std::string port_str = raw_uri.substr(raw_uri_pos_ + 1, port_end - raw_uri_pos_ - 1);
    std::pair<bool, unsigned short> port = utils::StrToNumericNoThrow<unsigned short>(port_str);
    if (!port.first) {
        status_ = URI_BAD_PORT_BIT;
        return;
    }
    port_ = port.second;
    raw_uri_pos_ = port_end;
    state_ = PS_PATH;
}

void Uri::ParsePath_(const std::string& raw_uri) {
    if (EndOfRawUri_(raw_uri) || raw_uri[raw_uri_pos_] != '/') {
        status_ = URI_BAD_PATH_BIT;
        return;
    }
    size_t path_end = raw_uri.find_first_of("?#", raw_uri_pos_);
    if (path_end == std::string::npos) {
        path_ = raw_uri.substr(raw_uri_pos_);
        state_ = PS_END;
        return;
    }
    path_ = raw_uri.substr(raw_uri_pos_, path_end - raw_uri_pos_);
    raw_uri_pos_ = path_end + 1;
    if (raw_uri[path_end] == '?') {
        state_ = PS_QUERY;
        return;
    }
    if (raw_uri[path_end] == '#') {
        state_ = PS_FRAGMENT;
        return;
    }
    state_ = PS_END; // should never reach here
}

void Uri::ParseQuery_(const std::string& raw_uri) {
    if (EndOfRawUri_(raw_uri)) {
        status_ = URI_BAD_QUERY_BIT;
        return;
    }
    size_t query_end = raw_uri.find_first_of("#", raw_uri_pos_);
    if (query_end == std::string::npos) {
        query_ = raw_uri.substr(raw_uri_pos_);
        state_ = PS_END;
        return;
    }
    query_ = raw_uri.substr(raw_uri_pos_, query_end - raw_uri_pos_);
    raw_uri_pos_ = query_end + 1;
    if (raw_uri[query_end] == '#') {
        state_ = PS_FRAGMENT;
        return;
    }
    state_ = PS_END; // should never reach here
}

void Uri::ParseFragment_(const std::string& raw_uri) {
    if (EndOfRawUri_(raw_uri)) {
        status_ = URI_BAD_FRAGMENT_BIT;
        return;
    }
    fragment_ = raw_uri.substr(raw_uri_pos_);
    state_ = PS_END;
}

//helpers:

bool Uri::EndOfRawUri_(const std::string& raw_uri) const {
    return raw_uri_pos_ >= raw_uri.size();
}

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
