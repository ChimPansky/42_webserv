#include <cerrno>
#include <cstdlib>
#include <cstring>

namespace utils {

const char* GetSystemErrorDescr()
{
    return strerror(errno);
}

void ExitWithCode(int exit_code)
{
    exit(exit_code);
}

}  // namespace utils
