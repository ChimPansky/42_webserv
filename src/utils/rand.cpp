#include <cstdlib>
#include <ctime>
#include <sstream>
#include <string>

namespace utils {

void InitRandomNumGenerator()
{
    srand(time(NULL));
}

std::string GenerateRandomString(unsigned len)
{
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    std::stringstream ss;

    // Generate a random filename until a unique one is found
    while (len--) {
        ss << charset[rand() % (sizeof(charset) - 1)];
    }

    return ss.str();
}

}  // namespace utils
