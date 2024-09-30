#include "gtest/gtest.h"
#include "../src/c_api/utils.cpp"
#include "arpa/inet.h"

int mult(int a, int b) {
    return a * b;
}

//TEST(TestSuiteName, TestName)
TEST(MultTest, Positive) {
    EXPECT_EQ(6, mult(2, 3));
    EXPECT_EQ(0, mult(0, 0));
    EXPECT_EQ(-6, mult(-2, 3));
    EXPECT_EQ(-6, mult(2, -3));
}

TEST(IPv4FromStringTest1, Positive) {
    EXPECT_EQ(ntohl(inet_addr("127.0.0.1")), c_api::IPv4FromString("127.0.0.1"));
    EXPECT_EQ(INADDR_LOOPBACK, c_api::IPv4FromString("localhost"));
    EXPECT_EQ((unsigned int)0, c_api::IPv4FromString("0.0.0.0"));
}

TEST(IPv4FromStringTest2, Negative) {
    std::string ip_str = "127.0.0.256";
    EXPECT_THROW(c_api::IPv4FromString(ip_str), std::runtime_error);
    EXPECT_NE(ntohl(inet_addr("192.168.1.1")), c_api::IPv4FromString("192.168.1.0"));
}

TEST(IPv4ToStringTest, Positive) {
    in_addr_t ip = inet_addr("127.0.0.1");
    EXPECT_EQ(inet_ntoa(*(in_addr*)&ip), c_api::IPv4ToString(htonl(ip)));
}

int main (int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
