#include "RqTarget.h"

#include <logger.h>
#include <numeric_utils.h>
#include <str_utils.h>

#include <cstring>

namespace http {

std::ostream& operator<<(std::ostream& out, const RqTarget& RqTarget)
{
    out << RqTarget.ToStr();
    return out;
}

// before decoding: check that each target-component only contains valid characters
const char* RqTarget::unreserved = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_.~";
const char* RqTarget::gen_delims = ":/?#[]@";
const char* RqTarget::sub_delims = "!$&'()*+,;=";

RqTarget::RqTarget(const std::string& raw_target) : validity_state_(TARGET_GOOD)
{
    size_t raw_target_pos = 0;
    ParseScheme_(raw_target, raw_target_pos);
    ParseUserInfo_(raw_target, raw_target_pos);
    ParseHost_(raw_target, raw_target_pos);
    ParsePort_(raw_target, raw_target_pos);
    ParsePath_(raw_target, raw_target_pos);
    ParseQuery_(raw_target, raw_target_pos);
    ParseFragment_(raw_target, raw_target_pos);
    std::pair<bool, std::string> decoded_str = PercentDecode_(path_.second, "/");
    if (!decoded_str.first) {
        validity_state_ |= TARGET_BAD_PATH;
        return;
    }
    path_.second = decoded_str.second;
    if (query_.first) {
        decoded_str = PercentDecode_(query_.second, "&");
        if (!decoded_str.first) {
            validity_state_ |= TARGET_BAD_QUERY;
            return;
        }
        query_.second = decoded_str.second;
    }
    std::pair<bool, std::string> normalized_path = RemoveDotSegments_(path_.second);
    if (!normalized_path.first) {
        validity_state_ |= TARGET_BAD_PATH;
        return;
    }
    path_.second = CollapseChars_(normalized_path.second, '/');
    // Validate_();
}

RqTarget::RqTarget(const std::string& scheme, const std::string& user_info, const std::string& host, const std::string& port, const std::string& path, const std::string query, const std::string& fragment)
{
    if (!scheme.empty()) {
        scheme_ = std::make_pair(true, scheme);
    }
    if (!user_info.empty()) {
        user_info_ = std::make_pair(true, user_info);
    }
    if (!host.empty()) {
        host_ = std::make_pair(true, host);
    }
    if (!port.empty()) {
        port_ = std::make_pair(true, port);
    }
    if (!path.empty()) {
        query_ = std::make_pair(true, path);
    }
    if (!query.empty()) {
        query_ = std::make_pair(true, query);
    }
    if (!fragment.empty()) {
        fragment_ = std::make_pair(true, fragment);
    }
    // Validate_(); // todo: check if valid and set state if error
}

RqTarget::RqTarget(const RqTarget& rhs)
{
    *this = rhs;
}

RqTarget& RqTarget::operator=(const RqTarget& rhs)
{
    if (this != &rhs) {
        scheme_ = rhs.scheme_;
        user_info_ = rhs.user_info_;
        host_ = rhs.host_;
        port_ = rhs.port_;
        path_ = rhs.path_;
        query_ = rhs.query_;
        fragment_ = rhs.fragment_;
        validity_state_ = rhs.validity_state_;
    }
    return *this;
}

bool RqTarget::operator==(const RqTarget& rhs) const
{
    return scheme_ == rhs.scheme_ && user_info_ == rhs.user_info_ && host_ == rhs.host_ && port_ == rhs.port_ && path_ == rhs.path_ && query_ == rhs.query_ && fragment_ == rhs.fragment_;
}

bool RqTarget::operator!=(const RqTarget& rhs) const
{
    return !(*this == rhs);
}

std::string RqTarget::ToStr() const
{
    std::string str;
    if (scheme_.first) {
        str += scheme_.second + "://";
    }
    if (user_info_.first) {
        str += user_info_.second + "@";
    }
    if (host_.first) {
        str += host_.second;
    }
    if (port_.first) {
        str += ":" + port_.second;
    }
    str += path_.second;
    if (query_.first) {
        str += "?" + query_.second;
    }
    if (fragment_.first) {
        str += "#" + fragment_.second;
    }
    return str;
}

void RqTarget::ParseScheme_(const std::string& raw_target, size_t& raw_target_pos)
{
    raw_target_pos = raw_target.find("://");
    if (raw_target_pos == std::string::npos) {
        scheme_.first = false;
        return;
    }
    scheme_.first = true;
    scheme_.second = utils::ToLowerCase(raw_target.substr(0, raw_target_pos));
    if (scheme_.second != "http") {
        validity_state_ |= TARGET_BAD_SCHEME;
        return;
    }
    raw_target_pos += 3;
}

void RqTarget::ParseUserInfo_(const std::string& raw_target, size_t& raw_target_pos)
{
    if (!Good()) {
        return;
    }
    size_t start_pos = raw_target_pos;
    raw_target_pos = raw_target.find("@", start_pos);
    if (raw_target_pos == std::string::npos) {
        user_info_.first = false;
        return;
    }
    user_info_.first = true;
    user_info_.second = raw_target.substr(start_pos, raw_target_pos - start_pos);
    validity_state_ |= TARGET_HAS_USER_INFO;
}


void RqTarget::ParseHost_(const std::string& raw_target, size_t& raw_target_pos)
{
    if (!Good() || !scheme_.first) {
        return;
    }
    size_t start_pos = raw_target_pos;
    raw_target_pos = raw_target.find_first_of(":/", start_pos);
    if (raw_target_pos == std::string::npos) {
        validity_state_ |= TARGET_BAD_PATH;
        return;
    }
    host_.first = true;
    host_.second = utils::ToLowerCase(raw_target.substr(start_pos, raw_target_pos - start_pos));
}

void RqTarget::ParsePort_(const std::string& raw_target, size_t& raw_target_pos)
{
    if (!Good() || !host_.first || raw_target_pos == std::string::npos || raw_target[raw_target_pos] != ':') {
        return;
    }
    raw_target_pos++;
    size_t start_pos = raw_target_pos;
    raw_target_pos = raw_target.find_first_of("/", raw_target_pos);
    if (raw_target_pos == std::string::npos) {
        validity_state_ = TARGET_BAD_PORT;
        return;
    }
    port_.first = true;
    port_.second = raw_target.substr(start_pos, raw_target_pos - start_pos);
}

void RqTarget::ParsePath_(const std::string& raw_target, size_t& raw_target_pos)
{
    if (!Good() || raw_target_pos == std::string::npos || raw_target[raw_target_pos] != '/') {
        validity_state_ = TARGET_BAD_PATH;
        return;
    }
    size_t start_pos = raw_target_pos;
    path_.first = true;
    raw_target_pos = raw_target.find_first_of("?#", start_pos);
    if (raw_target_pos == std::string::npos) {
        path_.second = raw_target.substr(start_pos);
        return;
    }
    path_.second = raw_target.substr(start_pos, raw_target_pos - start_pos);
}

void RqTarget::ParseQuery_(const std::string& raw_target, size_t& raw_target_pos)
{
    if (!Good() || raw_target_pos == std::string::npos || raw_target[raw_target_pos] != '?') {
        return;
    }
    query_.first = true;
    size_t start_pos = raw_target_pos + 1;
    raw_target_pos = raw_target.find_first_of("#", start_pos);
    if (raw_target_pos == std::string::npos) {
        query_.second = raw_target.substr(start_pos);
    }
    query_.second = raw_target.substr(start_pos, raw_target_pos - start_pos);
}

void RqTarget::ParseFragment_(const std::string& raw_target, size_t& raw_target_pos)
{
    if (!Good() || raw_target_pos == std::string::npos || raw_target[raw_target_pos] != '#') {
        return;
    }
    fragment_.first = true;
    fragment_.second = raw_target.substr(raw_target_pos + 1);
    validity_state_ |= TARGET_HAS_FRAGMENT;
}

std::pair<bool /*valid*/, std::string> RqTarget::PercentDecode_(const std::string& str,
                                                           const char* ignore_set) const
{
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

std::pair<bool /*valid*/, std::string> RqTarget::RemoveDotSegments_(const std::string& str) const
{
    int dir_level = 0;
    std::string input = str;
    std::string output;
    while (!input.empty()) {
        if (input.compare(0, 3, "../") == 0) {
            input.erase(0, 3);
            dir_level--;
        } else if (input.compare(0, 2, "./") == 0) {
            input.erase(0, 2);
        } else if (input == "/.." || input.compare(0, 4, "/../") == 0) {
            size_t erase_len = (input == "/.." ? 2 : 3);
            input.erase(1, erase_len);
            RemoveLastSegment_(output);
            dir_level--;
        } else if (input.compare(0, 3, "/./") == 0) {
            input.erase(1, 2);
        } else if (input == "/.") {
            input.erase(1, 1);
        } else if (input == ".") {
            input.clear();
        } else if (input == "..") {
            input.clear();
            dir_level--;
        } else {
            MoveSegmentToOutput_(input, output);
            dir_level++;
        }
        if (dir_level < 0) {
            return std::pair<bool, std::string>(false, "");
        }
    }
    return std::pair<bool, std::string>(true, output);
}

void RqTarget::RemoveLastSegment_(std::string& path) const
{
    size_t last_slash = path.find_last_of('/');
    if (last_slash == std::string::npos) {
        return;
    }
    path.erase(last_slash);
}

void RqTarget::MoveSegmentToOutput_(std::string& input, std::string& output) const
{
    size_t end_of_segment = input.find('/', 1);
    if (end_of_segment == std::string::npos) {
        output += input;
        input.clear();
    } else {
        output += input.substr(0, end_of_segment);
        input.erase(0, end_of_segment);
    }
}

std::string RqTarget::CollapseChars_(const std::string& str, char c) const
{
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

void RqTarget::Validate_()
{
    if (!Good()) {
        return;
    }
    if (!path_.first || !IsValidPath_(path_.second)) {
        validity_state_ |= TARGET_BAD_PATH;
    }
    if (query_.first && !IsValidQuery_(query_.second)) {
        validity_state_ |= TARGET_BAD_QUERY;
    }
    if (fragment_.first) {
        validity_state_ |= TARGET_HAS_FRAGMENT;
    }
}

// check valid characters
bool RqTarget::IsValidPath_(const std::string& path) const
{
    if (path.empty()) {
        return false;
    }
    for (size_t i = 0; i < path.size(); ++i) {
        if (!IsValidPathChar_(path[i])) {  // todo: further checks
            return false;
        }
    }
    return true;
}

bool RqTarget::IsValidQuery_(const std::string& query) const
{
    for (size_t i = 0; i < query.size(); ++i) {
        if (!IsValidQueryOrFragmentChar_(query[i])) {  // todo: further checks
            return false;
        }
    }
    return true;
}

bool RqTarget::IsValidPathChar_(char c) const
{
    return std::isalnum(c) || c == '.' || c == '-' || c == '_' || c == '/';
}

// very technically, we dont need to verify the query string, but just pass it on to cgi or
// wherever...
bool RqTarget::IsValidQueryOrFragmentChar_(char c) const
{
    (void)c;
    return true;  //
    // return std::isalnum(c) || c == '.' || c == '-' || c == '_' || c == '/' || c == '?' || c ==
    // '#' || c == '&';
}

}  // namespace http