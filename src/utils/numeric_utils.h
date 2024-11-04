#ifndef WS_UTILS_NUMERIC_UTILS_H
#define WS_UTILS_NUMERIC_UTILS_H

#include <limits>
#include <sstream>

namespace utils {

namespace {
template <typename NumType>
bool IsSignedType()
{
    return std::numeric_limits<NumType>::min() != 0;
}
}

template <typename NumType>
int StrToNumeric(const std::string& str)
{
    std::stringstream ss(str);
    NumType num;
    ss >> std::ws;  // throw here is there are spaces?
    std::string numstr = ss.str();
    if (numstr.empty() || (!IsSignedType<NumType>() && numstr[0] == '-')) {
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
    if (numstr.empty() || (!IsSignedType<NumType>() && numstr[0] == '-')) {
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


template <typename NumType>
std::pair<bool /* is_valid*/, NumType> HexToUnsignedNumericNoThrow(const std::string& str)
{
    NumType num;
    if (str.empty() || (IsSignedType<NumType>())) {
        return std::make_pair(false, 0);
    }
    if (!std::isxdigit(str[0])) {
        return std::make_pair(false, 0);
    }
    std::stringstream ss(str);
    ss >> std::hex >> num;
    if (ss.fail() || !ss.eof()) {
        return std::make_pair(false, 0);
    }
    return std::make_pair(true, num);
}


template <typename NumType>
std::string NumericToString(NumType num) {
    std::stringstream ss;
    ss << num;
    return ss.str();
}
}

#endif  // WS_UTILS_NUMERIC_UTILS_H


// #include <iostream>

// template <typename NumType>
// void test_one(std::string str) {
//     std::pair<bool, NumType> res = utils::HexToNumericNoThrow<size_t>(str);
//     std::cout << str << ": " << res.first << " " << res.second << std::endl;
// }

// #include <netinet/in.h>
// int main() {
//     test_one<size_t>(" a");
//     test_one<size_t>("a ");
//     test_one<size_t>("a");
//     test_one<size_t>("+a");
//     test_one<size_t>("-a");
//     test_one<size_t>("ab");
//     test_one<size_t>("ar");
//     test_one<size_t>("0");
//     test_one<size_t>("000");
    // test_one<int>("123");
    // test_one<int>("123a");
    // test_one<int>("-123");
    // test_one<int>("123.");
    // test_one<int>("123222222222222222222222222222222222222222");
    // test_one<int>("-123222222222222222222222222222222222222222");

    // test_one<unsigned short>("     001123");
    // test_one<unsigned short>("     0");
    // test_one<in_port_t>("     a0053");
    // test_one<unsigned short>("     0x1  ");
    // test_one<unsigned short>("0X1");
    // test_one<unsigned short>("-0X1");
    // test_one<unsigned short>("100000");

    // test_one<float>("1f");
    // test_one<float>("1.0");
    // test_one<float>("-50000000000000000");
    // test_one<float>("-5000000.0");
    // test_one<float>("a50");
    // test_one<float>("50a");
    // test_one<float>("a");

    // test_one<bool>("0");
    // test_one<bool>("100");
    // test_one<bool>("true");

// }