#ifndef WS_UTILS_NUMERIC_UTILS_H
#define WS_UTILS_NUMERIC_UTILS_H

#include <iomanip>
#include <limits>
#include <sstream>

#include "maybe.h"

namespace utils {

namespace {
template <typename NumType>
bool IsSignedType()
{
    return std::numeric_limits<NumType>::min() != 0;
}
}  // namespace

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
utils::maybe<NumType> StrToNumericNoThrow(const std::string& str)
{
    std::stringstream ss(str);
    NumType num;
    ss >> std::ws;
    std::string numstr = ss.str();
    if (numstr.empty() || (!IsSignedType<NumType>() && numstr[0] == '-')) {
        return maybe_not();
    }
    ss >> num;
    if (ss.fail()) {
        return maybe_not();
    }
    ss >> std::ws;
    if (!ss.eof()) {
        return maybe_not();
    }
    return num;
}


template <typename NumType>
utils::maybe<NumType> HexToUnsignedNumericNoThrow(const std::string& str)
{
    NumType num;
    if (str.empty() || (IsSignedType<NumType>())) {
        return utils::maybe_not();
    }
    if (!std::isxdigit(str[0])) {
        return utils::maybe_not();
    }
    std::stringstream ss(str);
    ss >> std::hex >> num;
    if (ss.fail() || !ss.eof()) {
        return utils::maybe_not();
    }
    return num;
}


template <typename NumType>
std::string NumericToString(NumType num)
{
    std::stringstream ss;
    ss << num;
    return ss.str();
}

template <typename NumType>
std::string NumericToHexStr(NumType c)
{
    std::stringstream ss;
    ss << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << (int)c;
    return ss.str();
}
}  // namespace utils

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
