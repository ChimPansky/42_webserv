#include "../src/http/SyntaxChecker.h"
#include "gtest/gtest.h"

TEST(TokenTests, 1_Test)
{
    std::string token = "abc";
    EXPECT_TRUE(http::SyntaxChecker::IsValidToken(token));
}

TEST(TokenTests, 2_Test)
{
    std::string token = "";
    EXPECT_FALSE(http::SyntaxChecker::IsValidToken(token));
}

TEST(TokenTests, 3_Test)
{
    std::string token = " ";
    EXPECT_FALSE(http::SyntaxChecker::IsValidToken(token));
}

TEST(TokenTests, 4_Test)
{
    std::string token = "abc ";
    EXPECT_FALSE(http::SyntaxChecker::IsValidToken(token));
}

TEST(TokenTests, 5_Test)
{
    std:: string token = " abc";
    EXPECT_FALSE(http::SyntaxChecker::IsValidToken(token));
}

TEST(TokenTests, 6_Test)
{
    std::string token = "token=invalid";
    EXPECT_FALSE(http::SyntaxChecker::IsValidToken(token));
}

TEST(TokenTests, 7_Test)
{
    std::string token = "\"";
    EXPECT_FALSE(http::SyntaxChecker::IsValidToken(token));
}

TEST(TokenTests, 8_Test)
{
    std::string token = "no?allowed";
    EXPECT_FALSE(http::SyntaxChecker::IsValidToken(token));
}

TEST(TokenTests, 9_Test)
{
    std::string token = "ws\tnot\vallowed";
    EXPECT_FALSE(http::SyntaxChecker::IsValidToken(token));
}

TEST(MethodTests, 1_Test)
{
    std::string method = "GET";
    EXPECT_TRUE(http::SyntaxChecker::IsValidMethod(method));
}

TEST(MethodTests, 2_Test)
{
    std::string method = "get";
    EXPECT_TRUE(http::SyntaxChecker::IsValidMethod(method));
}

TEST(MethodTests, 3_Test)
{
    std::string method = "wEiRdBuTst|11v4l1d~";
    EXPECT_TRUE(http::SyntaxChecker::IsValidMethod(method));
}

TEST(VersionTests, 1_Test)
{
    std::string version = "";
    EXPECT_FALSE(http::SyntaxChecker::IsValidVersion(version));
}

TEST(VersionTests, 2_Test)
{
    std::string version = " ";
    EXPECT_FALSE(http::SyntaxChecker::IsValidVersion(version));
}

TEST(VersionTests, 3_Test)
{
    std::string version = "HTTP/1.0";
    EXPECT_TRUE(http::SyntaxChecker::IsValidVersion(version));
}

TEST(VersionTests, 4_Test)
{
    std::string version = "HTTP/1.1";
    EXPECT_TRUE(http::SyntaxChecker::IsValidVersion(version));
}

TEST(VersionTests, 5_Test)
{
    std::string version = "\rHTTP/1.1";
    EXPECT_FALSE(http::SyntaxChecker::IsValidVersion(version));
}

TEST(VersionTests, 6_Test)
{
    std::string version = "HTTP/1.1\n";
    EXPECT_FALSE(http::SyntaxChecker::IsValidVersion(version));
}

TEST(VersionTests, 7_Test)
{
    std::string version = "HTTP/1.1 ";
    EXPECT_FALSE(http::SyntaxChecker::IsValidVersion(version));
}