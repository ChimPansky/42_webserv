#ifndef WS_UTILS_RAND_H
#define WS_UTILS_RAND_H

#include <string>

namespace utils {

void InitRandomNumGenerator();

std::string GenerateRandomString(unsigned len);

}  // namespace utils

#endif  // WS_UTILS_RAND_H
