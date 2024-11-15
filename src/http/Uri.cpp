#include "Uri.h"
#include <numeric_utils.h>
#include <logger.h>
#include <cstring>

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
    std::pair<bool, std::string> decoded_str = PercentDecode_(path_, "/");
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
    std::pair<bool, std::string> normalized_path = Normalize_(path_);
    if (!normalized_path.first) {
        validity_state_ |= URI_BAD_PATH_BIT;
        return;
    }
    path_ = CollapseChars_(normalized_path.second, '/');
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

std::pair<bool /*valid*/, std::string> Uri::PercentDecode_(const std::string& str, const char* ignore_set) const {
    std::string decoded;
    std::pair<bool, unsigned short> ascii;
    for (size_t i = 0; i < str.size(); ++i) {
        if (str[i] == '%') {
            if (i + 2 >= str.size()) {
                return std::pair<bool, std::string>(false, "");
            }
            ascii = utils::HexToUnsignedNumericNoThrow<unsigned short>(str.substr(i + 1, 2));
            if (!ascii.first) {
                return std::pair<bool, std::string>(false, "");
            }
            if (ignore_set && strchr(ignore_set, static_cast<char>(ascii.second))) {
                decoded += str.substr(i, 3);
                i += 2;
                continue;
            }
            decoded += static_cast<char>(ascii.second);
            i += 2;
        } else {
            decoded += str[i];
        }
    }
    return std::pair<bool, std::string>(true, decoded);
}

std::pair<bool /*valid*/, std::string> Uri::Normalize_(const std::string& str) const {
    int dir_level = 0;
    std::string input = str;
    std::string output;
    while (!input.empty()) {
        if (input.compare(0, 3, "../") == 0) {
            input.erase(0, 3);
            dir_level--;
        }
        else if (input.compare(0, 2, "./") == 0) {
            input.erase(0, 2);
        }
        else if (input == "/.."  || input.compare(0, 4, "/../") == 0) {
            size_t erase_len = (input == "/.." ? 2 : 3);
            input.erase(1, erase_len);
            RemoveLastSegment_(output);
            dir_level--;
        }
        else if (input.compare(0, 3, "/./") == 0) {
            input.erase(1, 2);
        }
        else if (input == "/.") {
            input.erase(1, 1);
        }
        else if (input == ".") {
            input.clear();
        }
        else if (input == "..") {
            input.clear();
            dir_level--;
        }
        else {
            MoveSegmentToOutput_(input, output);
            dir_level++;
        }
        if (dir_level < 0) {
            return std::pair<bool, std::string>(false, "");
        }
    }
    return std::pair<bool, std::string>(true, output);
}

void Uri::RemoveLastSegment_(std::string& path) const {
    size_t last_slash = path.find_last_of('/');
    if (last_slash == std::string::npos) {
        return;
    }
    path.erase(last_slash);
}

void Uri::MoveSegmentToOutput_(std::string& input, std::string& output) const {
    size_t end_of_segment = input.find('/', 1);
    if (end_of_segment == std::string::npos) {
        output += input;
        input.clear();
    }
    else {
        output += input.substr(0, end_of_segment);
        input.erase(0, end_of_segment);
    }
}

std::string Uri::CollapseChars_(const std::string& str, char c) const {
    std::string collapsed;
    for (size_t i = 0; i < str.size(); ++i) {
        if (str[i] == c) {
            if (i + 1 < str.size() && str[i + 1] == c) {
                continue;
            }
        }
        collapsed += str[i];
    }
    return collapsed;
}

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