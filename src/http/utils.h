#ifndef WS_HTTP_UTILS_H
#define WS_HTTP_UTILS_H

#include <sstream>

namespace http {

std::pair<bool /* is_valid*/, size_t> HexStrToSizeT(const std::string& hex_str);

}  // namespace http

#endif  // WS_HTTP_UTILS_H

// #include <iostream>

// template <typename NumType>
// void test_one(std::string str) {
//     std::pair<bool, NumType> res = utils::StrToNumericNoThrow<NumType>(str);
//     std::cout << str << ": " << res.first << " " << res.second << std::endl;
// }

// #include <netinet/in.h>
// int main() {
//     test_one<int>("123");
//     test_one<int>("123a");
//     test_one<int>("-123");
//     test_one<int>("123.");
//     test_one<int>("123222222222222222222222222222222222222222");
//     test_one<int>("-123222222222222222222222222222222222222222");

//     test_one<unsigned short>("     001123");
//     test_one<unsigned short>("     0");
//     test_one<in_port_t>("     a0053");
//     test_one<unsigned short>("     0x1  ");
//     test_one<unsigned short>("0X1");
//     test_one<unsigned short>("-0X1");
//     test_one<unsigned short>("100000");

//     test_one<float>("1f");
//     test_one<float>("1.0");
//     test_one<float>("-50000000000000000");
//     test_one<float>("-5000000.0");
//     test_one<float>("a50");
//     test_one<float>("50a");
//     test_one<float>("a");

//     test_one<bool>("0");
//     test_one<bool>("100");
//     test_one<bool>("true");

// }
