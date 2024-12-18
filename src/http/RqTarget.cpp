#include "RqTarget.h"

#include <http.h>
#include <str_utils.h>

#include <cstring>
#include <sstream>


namespace http {

std::ostream& operator<<(std::ostream& out, const RqTarget& RqTarget)
{
    out << RqTarget.ToStr();
    return out;
}


RqTarget::RqTarget() : validity_state_(RQ_TARGET_GOOD)
{}

RqTarget::RqTarget(const std::string& raw_target) : validity_state_(RQ_TARGET_GOOD)
{
    if (raw_target.size() > RQ_TARGET_LEN_LIMIT) {
        validity_state_ |= RQ_TARGET_TOO_LONG;
        return;
    }
    size_t raw_target_pos = 0;
    ParseScheme_(raw_target, raw_target_pos);
    ParseUserInfo_(raw_target, raw_target_pos);
    ParseHost_(raw_target, raw_target_pos);
    ParsePort_(raw_target, raw_target_pos);
    ParsePath_(raw_target, raw_target_pos);
    ParseQuery_(raw_target, raw_target_pos);
    ParseFragment_(raw_target, raw_target_pos);
    Validate_();
    Normalize_();
}

RqTarget::RqTarget(const std::string& scheme, const std::string& user_info, const std::string& host,
                   const std::string& port, const std::string& path, const std::string query,
                   const std::string& fragment)
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
        path_ = std::make_pair(true, path);
    }
    if (!query.empty()) {
        query_ = std::make_pair(true, query);
    }
    if (!fragment.empty()) {
        fragment_ = std::make_pair(true, fragment);
    }
    Validate_();
    Normalize_();
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
    return scheme_ == rhs.scheme_ && user_info_ == rhs.user_info_ && host_ == rhs.host_ &&
           port_ == rhs.port_ && path_ == rhs.path_ && query_ == rhs.query_ &&
           fragment_ == rhs.fragment_;
}

bool RqTarget::operator!=(const RqTarget& rhs) const
{
    return !(*this == rhs);
}

std::string RqTarget::ToStr() const
{
    std::stringstream ss;
    if (scheme_.first) {
        ss << scheme_.second << "://";
    }
    if (user_info_.first) {
        ss << user_info_.second << "@";
    }
    if (host_.first) {
        ss << host_.second;
    }
    if (port_.first) {
        ss << ":" << port_.second;
    }
    ss << http::PercentEncode(path_.second, "/");
    if (query_.first) {
        ss << "?" << http::PercentEncode(query_.second, "&=");
    }
    if (fragment_.first) {
        ss << "#" << fragment_.second;
    }
    return ss.str();
}

void RqTarget::AddTrailingSlashToPath()
{
    if (path_.first && path_.second[path_.second.size() - 1] != '/') {
        path_.second += "/";
    }
}

std::string RqTarget::GetDebugString() const
{
    std::ostringstream oss;

    oss << "DEBUG INFO FOR RequestTarget:\n";
    oss << "Good(): " << (Good() ? "YES" : "NO") << "\n";
    oss << "scheme defined: " << (scheme_.first ? "YES" : "NO") << "\n";
    oss << "scheme value: " << scheme() << "\n";
    oss << "user_info defined: " << (user_info_.first ? "YES" : "NO") << "\n";
    oss << "user_info value: " << user_info_.second << "\n";
    oss << "host defined: " << (host_.first ? "YES" : "NO") << "\n";
    oss << "host value: " << host_.second << "\n";
    oss << "port defined: " << (port_.first ? "YES" : "NO") << "\n";
    oss << "port value: " << port_.second << "\n";
    oss << "path defined: " << (path_.first ? "YES" : "NO") << "\n";
    oss << "path value: " << path_.second << "\n";
    oss << "query defined: " << (query_.first ? "YES" : "NO") << "\n";
    oss << "query value: " << query_.second << "\n";
    oss << "fragment defined: " << (fragment_.first ? "YES" : "NO") << "\n";
    oss << "fragment value: " << fragment_.second << "\n";
    oss << "FLAGS: "
        << "\n";
    oss << "RQ_TARGET_GOOD: " << (validity_state_ == RQ_TARGET_GOOD ? "YES" : "NO") << "\n";
    oss << "RQ_TARGET_TOO_LONG: " << (validity_state_ & RQ_TARGET_TOO_LONG ? "YES" : "NO") << "\n";
    oss << "RQ_TARGET_BAD_SCHEME: " << (validity_state_ & RQ_TARGET_BAD_SCHEME ? "YES" : "NO")
        << "\n";
    oss << "RQ_TARGET_HAS_USER_INFO: " << (validity_state_ & RQ_TARGET_HAS_USER_INFO ? "YES" : "NO")
        << "\n";
    oss << "RQ_TARGET_BAD_HOST: " << (validity_state_ & RQ_TARGET_BAD_HOST ? "YES" : "NO") << "\n";
    oss << "RQ_TARGET_BAD_PORT: " << (validity_state_ & RQ_TARGET_BAD_PORT ? "YES" : "NO") << "\n";
    oss << "RQ_TARGET_BAD_PATH: " << (validity_state_ & RQ_TARGET_BAD_PATH ? "YES" : "NO") << "\n";
    oss << "RQ_TARGET_BAD_QUERY: " << (validity_state_ & RQ_TARGET_BAD_QUERY ? "YES" : "NO")
        << "\n";
    oss << "RQ_TARGET_HAS_FRAGMENT: " << (validity_state_ & RQ_TARGET_HAS_FRAGMENT ? "YES" : "NO")
        << "\n";
    oss << "ToStr(): " << ToStr() << std::endl;

    return oss.str();
}

void RqTarget::ParseScheme_(const std::string& raw_target, size_t& raw_target_pos)
{
    size_t end_pos = raw_target.find("://");
    if (end_pos == std::string::npos) {
        scheme_.first = false;
        return;
    }
    scheme_.first = true;
    scheme_.second = utils::ToLowerCase(raw_target.substr(0, end_pos));
    raw_target_pos = end_pos + 3;
}

void RqTarget::ParseUserInfo_(const std::string& raw_target, size_t& raw_target_pos)
{
    if (!scheme_.first || raw_target_pos >= raw_target.size()) {
        return;
    }
    size_t start_pos = raw_target_pos;
    size_t end_pos = raw_target.find("@", start_pos);
    ;
    if (end_pos == std::string::npos) {
        user_info_.first = false;
        return;
    }
    user_info_.first = true;
    user_info_.second = raw_target.substr(start_pos, end_pos - start_pos);
    raw_target_pos = end_pos + 1;
}

void RqTarget::ParseHost_(const std::string& raw_target, size_t& raw_target_pos)
{
    if (!scheme_.first || raw_target_pos >= raw_target.size()) {
        return;
    }
    host_.first = true;
    size_t start_pos = raw_target_pos;
    raw_target_pos = raw_target.find_first_of(":/", start_pos);
    if (raw_target_pos == std::string::npos) {
        host_.second = utils::ToLowerCase(raw_target.substr(start_pos));
        return;
    }
    host_.second = utils::ToLowerCase(raw_target.substr(start_pos, raw_target_pos - start_pos));
}

void RqTarget::ParsePort_(const std::string& raw_target, size_t& raw_target_pos)
{
    if (!host_.first || raw_target_pos >= raw_target.size() || raw_target[raw_target_pos] != ':') {
        return;
    }
    port_.first = true;
    raw_target_pos++;
    size_t start_pos = raw_target_pos;
    raw_target_pos = raw_target.find_first_of("/?#", raw_target_pos);
    if (raw_target_pos == std::string::npos) {
        port_.second = raw_target.substr(start_pos);
        return;
    }
    port_.second = raw_target.substr(start_pos, raw_target_pos - start_pos);
}

void RqTarget::ParsePath_(const std::string& raw_target, size_t& raw_target_pos)
{
    if (raw_target_pos >= raw_target.size() || raw_target[raw_target_pos] != '/') {
        validity_state_ = RQ_TARGET_BAD_PATH;
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
    if (raw_target_pos >= raw_target.size() || raw_target[raw_target_pos] != '?') {
        return;
    }
    query_.first = true;
    size_t start_pos = raw_target_pos + 1;
    raw_target_pos = raw_target.find("#", start_pos);
    if (raw_target_pos == std::string::npos) {
        query_.second = raw_target.substr(start_pos);
    }
    query_.second = raw_target.substr(start_pos, raw_target_pos - start_pos);
}

void RqTarget::ParseFragment_(const std::string& raw_target, size_t& raw_target_pos)
{
    if (raw_target_pos >= raw_target.size() || raw_target[raw_target_pos] != '#') {
        return;
    }
    fragment_.first = true;
    fragment_.second = raw_target.substr(raw_target_pos + 1);
}

void RqTarget::Normalize_()
{
    if (host_.first) {
        std::pair<bool, std::string> decoded = http::PercentDecode(host_.second);
        if (decoded.first) {
            host_.second = decoded.second;
        } else {  // invalid encoding detected -> BAD_REQUEST
            validity_state_ |= RQ_TARGET_BAD_HOST;
        }
        ConvertEncodedHexToUpper_(host_.second);
    }
    if (port_.first && port_.second == "80" && scheme_.second == "http") {
        port_ = std::pair<bool, std::string>(false, "");
    }
    if (path_.first) {
        std::pair<bool, std::string> decoded = http::PercentDecode(path_.second, "/");
        if (decoded.first) {
            path_.second = decoded.second;
        } else {  // invalid encoding detected -> BAD_REQUEST
            validity_state_ |= RQ_TARGET_BAD_PATH;
        }
        ConvertEncodedHexToUpper_(path_.second);
        path_.second = CollapseSlashes_(path_.second);
        std::pair<bool, std::string> dot_segment_free_path = RemoveDotSegments_(path_.second);
        if (dot_segment_free_path.first) {
            path_.second = dot_segment_free_path.second;
        } else {  // directory traversal detected -> BAD_REQUEST (log it?)
            validity_state_ |= RQ_TARGET_BAD_PATH;
        }
    }
    if (query_.first) {
        std::pair<bool, std::string> decoded =
            http::PercentDecode(query_.second, "&=");  // maybe change to "&=+,/,;?"
        if (decoded.first) {
            query_.second = decoded.second;
        } else {  // invalid encoding detected -> BAD_REQUEST
            validity_state_ |= RQ_TARGET_BAD_QUERY;
        }
        ConvertEncodedHexToUpper_(query_.second);
    }
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

std::string RqTarget::CollapseSlashes_(const std::string& str) const
{
    std::string collapsed;
    for (size_t i = 0; i < str.size(); ++i) {
        if (str[i] == '/') {
            if (i > 0 && i + 1 < str.size() && str[i + 1] == '/') {
                continue;
            }
        }
        collapsed += str[i];
    }
    return collapsed;
}

void RqTarget::ConvertEncodedHexToUpper_(std::string& str)
{
    for (size_t i = 0; i < str.size(); ++i) {
        if (str[i] == '%' && i + 2 < str.size()) {
            str[i + 1] = std::toupper(str[i + 1]);
            str[i + 2] = std::toupper(str[i + 2]);
        }
    }
}

void RqTarget::Validate_()
{
    if (scheme_.first) {
        ValidateScheme_();
    }
    if (user_info_.first) {
        validity_state_ |= RQ_TARGET_HAS_USER_INFO;
    }
    if (host_.first) {
        ValidateHost_();
    }
    if (port_.first) {
        ValidatePort_();
    }
    if (path_.first) {
        ValidatePath_();
    }
    if (query_.first) {
        ValidateQuery_();
    }
    if (fragment_.first) {
        validity_state_ |= RQ_TARGET_HAS_FRAGMENT;
    }
}

void RqTarget::ValidateScheme_()
{
    if (!scheme_.first) {
        return;
    }
    if (scheme_.second != "http") {
        validity_state_ |= RQ_TARGET_BAD_SCHEME;
    }
}

void RqTarget::ValidateHost_()
{
    // todo: decide if we check for valid ipv4 or ipv6 address,...
    if (!host_.first) {
        return;
    }
    if (!scheme_.first) {
        validity_state_ |= RQ_TARGET_BAD_SCHEME;
        return;
    }
    if (host_.second.empty()) {
        validity_state_ |= RQ_TARGET_BAD_HOST;
    }
    for (size_t i = 0; i < host_.second.size(); ++i) {
        const char* str = host_.second.c_str() + i;
        if (IsEncodedOctet_(str)) {
            i += 2;
            continue;
        }
        if (std::strchr(kUnreserved, *str) != NULL) {
            continue;
        }
        validity_state_ |= RQ_TARGET_BAD_HOST;
    }
}

void RqTarget::ValidatePort_()
{
    if (!port_.first) {
        return;
    }
    if (!scheme_.first) {
        validity_state_ |= RQ_TARGET_BAD_SCHEME;
        return;
    }
    if (!host_.first) {
        validity_state_ |= RQ_TARGET_BAD_HOST;
        return;
    }
    if (port_.second.empty() || !utils::StrToNumericNoThrow<unsigned short>(port_.second).first) {
        validity_state_ |= RQ_TARGET_BAD_PORT;
    }
}

// assume that path is already normalized and (unreserved)-decoded
void RqTarget::ValidatePath_()
{
    if (!path_.first || path_.second.empty() || path_.second[0] != '/') {
        validity_state_ |= RQ_TARGET_BAD_PATH;
        return;
    }
    for (size_t i = 0; i < path_.second.size(); ++i) {
        const char* str = path_.second.c_str() + i;
        if (i == 1 && *str == '/') {
            validity_state_ |= RQ_TARGET_BAD_PATH;
            return;
        }
        if (IsEncodedOctet_(str)) {
            i += 2;
            continue;
        }
        if (std::strchr(kUnreserved, *str) != NULL || std::strchr(kSubDelims, *str) ||
            std::strchr(":@/", *str) != NULL) {
            continue;
        }
        validity_state_ |= RQ_TARGET_BAD_PATH;
    }
}

void RqTarget::ValidateQuery_()
{
    if (!query_.first) {
        return;
    }
    for (size_t i = 0; i < query_.second.size(); ++i) {
        const char* str = query_.second.c_str() + i;
        if (IsEncodedOctet_(str)) {
            i += 2;
            continue;
        }
        if (std::strchr(kUnreserved, *str) != NULL || std::strchr(kSubDelims, *str) ||
            std::strchr(":@/?", *str) != NULL) {
            continue;
        }
        validity_state_ |= RQ_TARGET_BAD_QUERY;
        return;
    }
}

bool RqTarget::IsEncodedOctet_(const char* str) const
{
    if (*str != '%') {
        return false;
    }
    if (str[1] == '\0' || str[2] == '\0') {
        return false;
    }
    if (!std::isxdigit(str[1]) || !std::isxdigit(str[2])) {
        return false;
    }
    return true;
}

}  // namespace http
