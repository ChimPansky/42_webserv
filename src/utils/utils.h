#ifndef WS_UTILS_UTILS_H
#define WS_UTILS_UTILS_H

#include <dirent.h>
#include <netdb.h>

#include <limits>
#include <sstream>
#include <vector>

namespace utils {

std::string ToLowerCase(const std::string& input);

namespace fs {

std::vector<std::string> SplitLine(const std::string& line);
bool CheckFileExtension(const std::string& file, const std::string& extention);
bool ValidPath(const std::string& val);
bool IsDirectory(const std::string& path);
std::string Trim(const std::string& str, const std::string& trim_chars);

}  // namespace fs

namespace detail {

template <typename NumType>
bool IsSignedType()
{
    return std::numeric_limits<NumType>::min() != 0;
}

}  // namespace detail

template <typename NumType>
int StrToNumeric(const std::string& str)
{
    std::stringstream ss(str);
    NumType num;
    ss >> std::ws;  // throw here is there are spaces?
    std::string numstr = ss.str();
    if (numstr.empty() || (!detail::IsSignedType<NumType>() && numstr[0] == '-')) {
        throw std::invalid_argument("cannot parse int: " + str);
    }
    ss >> num;
    if (ss.fail()) {
        throw std::invalid_argument("cannot parse int: " + str);
    }
    ss >> std::ws;  // throw here is there are spaces?
    if (!ss.eof()) {
        throw std::invalid_argument("cannot parse int: " + str);
    }
    return num;
}

template <typename NumType>
std::pair<bool /*is_valid*/, NumType> StrToNumericNoThrow(const std::string& str)
{
    std::stringstream ss(str);
    NumType num;
    ss >> std::ws;
    std::string numstr = ss.str();
    if (numstr.empty() || (!detail::IsSignedType<NumType>() && numstr[0] == '-')) {
        return std::make_pair(false, 0);
    }
    ss >> num;
    if (ss.fail()) {
        return std::make_pair(false, 0);
    }
    ss >> std::ws;
    if (!ss.eof()) {
        return std::make_pair(false, 0);
    }
    return std::make_pair(true, num);
}

}  // namespace utils

#endif  // WS_UTILS_UTILS_H
