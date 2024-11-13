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
    size_t raw_uri_pos = 0;
    ParsePath_(raw_uri, raw_uri_pos);
    ParseQuery_(raw_uri, raw_uri_pos);
    ParseFragment_(raw_uri, raw_uri_pos);
    std::pair<bool, std::string> decoded_str = PercentDecode_(path_);
    if (!decoded_str.first) {
        validity_state_ |= URI_BAD_PATH_BIT;
        return;
    }
    path_ = decoded_str.second;

    if (query_.first) {
        decoded_str = PercentDecode_(query_.second);
        if (!decoded_str.first) {
            validity_state_ |= URI_BAD_QUERY_BIT;
            return;
        }
        query_.second = decoded_str.second;
    }

    if (fragment_.first) {
        decoded_str = PercentDecode_(fragment_.second);
        if (!decoded_str.first) {
            validity_state_ |= URI_BAD_FRAGMENT_BIT;
            return;
        }
        fragment_.second = decoded_str.second;
    }


    // Decode();
    // Normalize();
    //Validate_();
}

Uri::Uri(const std::string& path, const std::string& query, const std::string& fragment) : path_(path) {
    if (!query.empty()) {
        query_ = std::make_pair(true, query);
    }
    if (!fragment.empty()) {
        fragment_ = std::make_pair(true, fragment);
    }
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
    if (query_.first) {
        str += "?" + query_.second;
    }
    if (fragment_.first) {
        str += "#" + fragment_.second;
    }
    return str;
}

void Uri::ParsePath_(const std::string& raw_uri, size_t& raw_uri_pos) {
    if (raw_uri_pos >= raw_uri.size() || raw_uri[raw_uri_pos] != '/') {
        validity_state_ = URI_BAD_PATH_BIT;
        return;
    }
    size_t start_pos = raw_uri_pos;
    raw_uri_pos = raw_uri.find_first_of("?#", start_pos);
    if (raw_uri_pos == std::string::npos) {
        path_ = raw_uri.substr(start_pos);
        return;
    }
    path_ = raw_uri.substr(start_pos, raw_uri_pos - start_pos);
}

void Uri::ParseQuery_(const std::string& raw_uri, size_t& raw_uri_pos) {
    if (!Good() || raw_uri_pos >= raw_uri.size() || raw_uri[raw_uri_pos] != '?') {
        return;
    }
    query_.first = true;
    size_t start_pos = raw_uri_pos + 1;
    raw_uri_pos = raw_uri.find_first_of("#", start_pos);
    if (raw_uri_pos == std::string::npos) {
        query_.second = raw_uri.substr(start_pos);
    }
    query_.second = raw_uri.substr(start_pos, raw_uri_pos - start_pos);
}

void Uri::ParseFragment_(const std::string& raw_uri, size_t& raw_uri_pos) {
    if (!Good() || raw_uri_pos >= raw_uri.size() || raw_uri[raw_uri_pos] != '#') {
        return;
    }
    fragment_.first = true;
    fragment_.second = raw_uri.substr(raw_uri_pos + 1);
}

std::pair<bool /*valid*/, std::string> Uri::PercentDecode_(const std::string& str) const {
    std::string decoded;
    std::pair<bool, unsigned short> ascii;
    for (size_t i = 0; i < str.size(); ++i) {
        if (str[i] == '%') {
            if (i + 2 >= str.size()) {
                return std::pair<bool, std::string>(false, "");
            }
            ascii = utils::HexToUnsignedNumericNoThrow<unsigned short>(str.substr(i + 1, 2));
            if (!ascii.first) {
                LOG(DEBUG) << "Uri::PercentDecode_: bad hex: " << str.substr(i + 1, 2);
                return std::pair<bool, std::string>(false, "");
            }
            decoded += static_cast<char>(ascii.second);
            i += 2;
        } else {
            decoded += str[i];
        }
    }
    return std::pair<bool, std::string>(true, decoded);
}

// Normalize():
//    1.  The input buffer is initialized with the now-appended path
//        components and the output buffer is initialized to the empty
//        string.

//    2.  While the input buffer is not empty, loop as follows:

//        A.  If the input buffer begins with a prefix of "../" or "./",
//            then remove that prefix from the input buffer; otherwise,

//        B.  if the input buffer begins with a prefix of "/./" or "/.",
//            where "." is a complete path segment, then replace that
//            prefix with "/" in the input buffer; otherwise,

//        C.  if the input buffer begins with a prefix of "/../" or "/..",
//            where ".." is a complete path segment, then replace that
//            prefix with "/" in the input buffer and remove the last
//            segment and its preceding "/" (if any) from the output
//            buffer; otherwise,

//        D.  if the input buffer consists only of "." or "..", then remove
//            that from the input buffer; otherwise,

//        E.  move the first path segment in the input buffer to the end of
//            the output buffer, including the initial "/" character (if
//            any) and any subsequent characters up to, but not including,
//            the next "/" character or the end of the input buffer.

//    3.  Finally, the output buffer is returned as the result of
//        remove_dot_segments.

void Uri::Validate_() {
    if (!Good()) {
        return;
    }
    if (!IsValidPath_(path_)) {
        validity_state_ |= URI_BAD_PATH_BIT;
    }
    if (query_.first && !IsValidQuery_(query_.second)) {
        validity_state_ |= URI_BAD_QUERY_BIT;
    }
    if (fragment_.first && !IsValidFragment_(fragment_.second)) {
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
