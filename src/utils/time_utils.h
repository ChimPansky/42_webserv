#ifndef WS_UTILS_TIME_UTILS_H
#define WS_UTILS_TIME_UTILS_H

#include <string>

typedef long UnixTimestampS;

namespace utils {

// TODO: 0 is a valid timestamp, bad value for default;
std::string GetFormatedTime(UnixTimestampS raw_time = 0);

UnixTimestampS Now();

}  // namespace utils

#endif  // WS_UTILS_TIME_UTILS_H
