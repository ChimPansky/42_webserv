#ifndef WS_UTILS_ERRORS_H
#define WS_UTILS_ERRORS_H

namespace utils {
const char* GetSystemErrorDescr();
void ExitWithCode(int exit_code);
}  // namespace utils


#endif  // WS_UTILS_ERRORS_H
