#include <vector>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <sstream>

// void process(std::vector<char>& vec) {
//     vec_print()
//     vec.erase(vec.begin(), vec.begin() + 5);
// }

void shift_left(std::vector<char>& vec, size_t n) {
    vec.erase(vec.begin(), vec.begin() + n);
}

void vec_print(const std::vector<char>& vec) {
    for (size_t i = 0; i < vec.size(); i++) {
        std::cout << vec[i];
    }
    std::cout << std::endl;
}

ssize_t receive(int fd, std::stringstream& ss, size_t sz) {
    char buf[sz];
    ssize_t bytes_read = read(fd, buf, sz);
    if (bytes_read >= 0) {
        ss << buf;
    }
    return bytes_read;
}

void process_lines(std::string& line, std::stringstream& ss) {
    while (std::getline(ss, line, '\n')) {
        std::cout << "LINE: " << line << std::endl;
        std::cout << "LINE SIZE: " << line.size() << std::endl;
        std::cout << "LAST CHAR: " << (int)line[line.size() - 1] << std::endl;
    }
}

int main() {

    // std::deque<char> deq;
    size_t READ_SZ = 10;
    int fd = open("rq1.txt", O_RDONLY);
    std::stringstream ss;
    while (receive(fd, ss, READ_SZ) > 0) {
        std::string line;
        process_lines(line, ss);
    }



    // vec_print(vec);
    // shift_left(vec, 5);
    // vec_print(vec);

    // deq.insert(deq.begin(), buf, buf + 10);
    // std::copy(buf)

}