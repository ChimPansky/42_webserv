#include "Uri.h"
#include <http.h>
#include <numeric_utils.h>
#include <logger.h>

namespace http {


std::ostream& operator<<(std::ostream& out, const Uri& uri) {
    out << uri.ToStr();
    return out;
}

Uri::Uri(const std::string& raw_uri) : validity_state_(URI_GOOD_BIT) {
    ParseState state = PS_PATH;
    size_t raw_uri_pos = 0;
    while (state != PS_END && validity_state_ == URI_GOOD_BIT) {
        switch (state) {
            case PS_PATH: ParsePath_(raw_uri, raw_uri_pos, state); break;
            case PS_QUERY: ParseQuery_(raw_uri, raw_uri_pos, state); break;
            case PS_FRAGMENT: ParseFragment_(raw_uri, raw_uri_pos, state); break;
            case PS_END: break;
        }
    }
    Validate_();
}

Uri::Uri(const std::string& path, const std::string& query, const std::string& fragment)
    : path_(path), query_(query), fragment_(fragment) {
    Validate_(); // todo: check if valid and set state if error
}

Uri::Uri(const Uri& rhs) {
    *this = rhs;
}

Uri& Uri::operator=(const Uri& rhs) {
    if (this != &rhs) {
        path_ = rhs.path_;
        query_ = rhs.query_;
        fragment_ = rhs.fragment_;
        validity_state_ = rhs.validity_state_;
    }
    return *this;
}

bool Uri::operator==(const Uri& rhs) const {
    return path_ == rhs.path_ && query_ == rhs.query_ && fragment_ == rhs.fragment_;
}

bool Uri::operator!=(const Uri& rhs) const{
    return !(*this == rhs);
}

std::string Uri::ToStr() const {
    std::string str;
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

void Uri::ParsePath_(const std::string& raw_uri, size_t& raw_uri_pos, ParseState& state) {
    if (raw_uri_pos >= raw_uri.size() || raw_uri[raw_uri_pos] != '/') {
        validity_state_ = URI_BAD_PATH_BIT;
        return;
    }
    size_t path_end = raw_uri.find_first_of("?#", raw_uri_pos);
    if (path_end == std::string::npos) {
        path_ = raw_uri.substr(raw_uri_pos);
        state = PS_END;
        return;
    }
    path_ = raw_uri.substr(raw_uri_pos, path_end - raw_uri_pos);
    raw_uri_pos = path_end + 1;
    if (raw_uri[path_end] == '?') {
        state = PS_QUERY;
        return;
    }
    if (raw_uri[path_end] == '#') {
        state = PS_FRAGMENT;
        return;
    }
    throw std::logic_error("Error in path-parsing logic in http::Uri");
}

void Uri::ParseQuery_(const std::string& raw_uri, size_t& raw_uri_pos, ParseState& state) {
    if (raw_uri_pos >= raw_uri.size()) {
        validity_state_ = URI_BAD_QUERY_BIT;
        return;
    }
    size_t query_end = raw_uri.find_first_of("#", raw_uri_pos);
    if (query_end == std::string::npos) {
        query_ = raw_uri.substr(raw_uri_pos);
        state = PS_END;
        return;
    }
    query_ = raw_uri.substr(raw_uri_pos, query_end - raw_uri_pos);
    raw_uri_pos = query_end + 1;
    if (raw_uri[query_end] == '#') {
        state = PS_FRAGMENT;
        return;
    }
    throw std::logic_error("Error in query-parsing logic in http::Uri");
}

void Uri::ParseFragment_(const std::string& raw_uri, size_t& raw_uri_pos, ParseState& state) {
    if (raw_uri_pos >= raw_uri.size()) {
        validity_state_ = URI_BAD_FRAGMENT_BIT;
        return;
    }
    fragment_ = raw_uri.substr(raw_uri_pos);
    state = PS_END;
}

void Uri::Validate_() {
    if (!IsValidPath_(path_)) {
        validity_state_ |= URI_BAD_PATH_BIT;
    }
    if (!IsValidQuery_(query_)) {
        validity_state_ |= URI_BAD_QUERY_BIT;
    }
    if (!IsValidFragment_(fragment_)) {
        validity_state_ |= URI_BAD_FRAGMENT_BIT;
    }
}

// check valid characters
// check if .. leads goes 1 level above root of path...
bool Uri::IsValidPath_(const std::string& path) const {
    if (path.empty()) {
        return false;
    }
    for (size_t i = 0; i < path.size(); ++i) {
        if (!IsValidPathChar_(path[i])) {   //todo: further checks
            return false;
        }
    }
    return true;
}

bool Uri::IsValidQuery_(const std::string& query) const { ////
    for (size_t i = 0; i < query.size(); ++i) {
        if (!IsValidQueryOrFragmentChar_(query[i])) {   //todo: further checks
            return false;
        }
    }
    return true;
}

bool Uri::IsValidFragment_(const std::string &fragment) const {
    for (size_t i = 0; i < fragment.size(); ++i) {
        if (!IsValidQueryOrFragmentChar_(fragment[i])) {
            return false;
        }
    }
    return true;
}

bool Uri::IsValidPathChar_(char c) const {
    return std::isalnum(c) || c == '.' || c == '-' || c == '_' || c == '/';
}

// very technically, we dont need to verify the query string, but just pass it on to cgi or wherever...
bool Uri::IsValidQueryOrFragmentChar_(char c) const {
    (void)c;
    return true; //
    // return std::isalnum(c) || c == '.' || c == '-' || c == '_' || c == '/' || c == '?' || c == '#' || c == '&';
}

} // namespace http
