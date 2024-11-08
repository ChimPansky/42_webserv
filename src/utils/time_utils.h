#ifndef WS_UTILS_TIME_UTILS_H
#define WS_UTILS_TIME_UTILS_H

#include <ctime>
#include <string>

namespace utils {

std::string GetFormatedTime(std::time_t raw_time = 0);

}

#endif  // WS_UTILS_TIME_UTILS_H
