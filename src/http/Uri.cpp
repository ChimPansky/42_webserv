#include "Uri.h"
#include <http.h>
#include <numeric_utils.h>
#include <logger.h>

namespace http {

Uri::Uri(const std::string& raw_uri) : status_(URI_GOOD_BIT), path_(""), query_(""), fragment_("") {
    // parse raw_uri
    ParseRawUri_(raw_uri);
}

Uri::Uri(const std::string& path, const std::string& query, const std::string& fragment)
    : path_(path), query_(query), fragment_(fragment) {
    Validate_(); // todo: check if valid and set state if error
}

Uri::Uri(const Uri& other) {
    *this = other;
}

Uri& Uri::operator=(const Uri& other) {
    if (this != &other) {
        path_ = other.path_;
        query_ = other.query_;
        fragment_ = other.fragment_;
        status_ = other.status_;
    }
    return *this;
}

bool Uri::operator==(const Uri& other) const {
    return path_ == other.path_ && query_ == other.query_ && fragment_ == other.fragment_;
}

bool Uri::operator!=(const Uri& other) const {
    return !(*this == other);
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


void Uri::ParseRawUri_(const std::string& raw_uri) {
    // host_ = raw_uri;
    // port_ = 0;
    //todo: go through raw_uri and parse
    ParseState state = PS_PATH;
    size_t raw_uri_pos = 0;
    while (state != PS_END && status_ == URI_GOOD_BIT) {
        switch (state) {
            case PS_PATH: ParsePath_(raw_uri, raw_uri_pos, state); break;
            case PS_QUERY: ParseQuery_(raw_uri, raw_uri_pos, state); break;
            case PS_FRAGMENT: ParseFragment_(raw_uri, raw_uri_pos, state); break;
            case PS_END: break;
        }
    }
    Validate_();
}

void Uri::ParsePath_(const std::string& raw_uri, size_t& raw_uri_pos, ParseState& state) {
    if (raw_uri_pos >= raw_uri.size() || raw_uri[raw_uri_pos] != '/') {
        status_ = URI_BAD_PATH_BIT;
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
    state = PS_END; // should never reach here
}

void Uri::ParseQuery_(const std::string& raw_uri, size_t& raw_uri_pos, ParseState& state) {
    LOG(DEBUG) << "raw_uri_pos: " << raw_uri_pos << " raw_uri.size(): " << raw_uri.size();
    if (raw_uri_pos >= raw_uri.size()) {
        status_ = URI_BAD_QUERY_BIT;
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
    state = PS_END; // should never reach here
}

void Uri::ParseFragment_(const std::string& raw_uri, size_t& raw_uri_pos, ParseState& state) {
    if (raw_uri_pos >= raw_uri.size()) {
        status_ = URI_BAD_FRAGMENT_BIT;
        return;
    }
    fragment_ = raw_uri.substr(raw_uri_pos);
    state = PS_END;
}

void Uri::Validate_() {
    LOG(DEBUG) << "Validate()";
    if (!IsValidPath_(path_)) {
        status_ |= URI_BAD_PATH_BIT;
    }
    if (!IsValidQuery_(query_)) {
        status_ |= URI_BAD_QUERY_BIT;
        LOG(DEBUG) << "bad_query_bit: " << URI_BAD_QUERY_BIT;
    }
    if (!IsValidFragment_(fragment_)) {
        status_ |= URI_BAD_FRAGMENT_BIT;
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
            LOG(DEBUG) << "Query invalid!";
            return false;
        }
    }
    LOG(DEBUG) << "Query valid!";
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

bool Uri::IsValidPathChar_(char c) const {
    return std::isalnum(c) || c == '.' || c == '-' || c == '_' || c == '/';
}

bool Uri::IsValidQueryOrFragmentChar_(char c) const {
    return std::isalnum(c) || c == '.' || c == '-' || c == '_' || c == '/' || c == '?' || c == '#' || c == '&';
}

} // namespace http
