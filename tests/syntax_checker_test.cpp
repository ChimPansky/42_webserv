#include "../src/http/SyntaxChecker.h"
#include "gtest/gtest.h"

TEST(TokenTests, 1_Test)
{
    std::string token = "abc";
    EXPECT_TRUE(http::SyntaxChecker::IsValidTokenName(token));
}

TEST(TokenTests, 2_Test)
{
    std::string token = "";
    EXPECT_FALSE(http::SyntaxChecker::IsValidTokenName(token));
}

TEST(TokenTests, 3_Test)
{
    std::string token = " ";
    EXPECT_FALSE(http::SyntaxChecker::IsValidTokenName(token));
}

TEST(TokenTests, 4_Test)
{
    std::string token = "abc ";
    EXPECT_FALSE(http::SyntaxChecker::IsValidTokenName(token));
}

TEST(TokenTests, 5_Test)
{
    std:: string token = " abc";
    EXPECT_FALSE(http::SyntaxChecker::IsValidTokenName(token));
}

TEST(TokenTests, 6_Test)
{
    std::string token = "token=invalid";
    EXPECT_FALSE(http::SyntaxChecker::IsValidTokenName(token));
}

TEST(TokenTests, 7_Test)
{
    std::string token = "\"";
    EXPECT_FALSE(http::SyntaxChecker::IsValidTokenName(token));
}

TEST(TokenTests, 8_Test)
{
    std::string token = "no?allowed";
    EXPECT_FALSE(http::SyntaxChecker::IsValidTokenName(token));
}

TEST(TokenTests, 9_Test)
{
    std::string token = "ws\tnot\vallowed";
    EXPECT_FALSE(http::SyntaxChecker::IsValidTokenName(token));
}

TEST(TokenTests, 10_Test)
{
    std::string token = "wEiRdBuTst|11v4l1d~";
    EXPECT_TRUE(http::SyntaxChecker::IsValidTokenName(token));
}

TEST(VersionTests, 1_Test)
{
    std::string version = "";
    EXPECT_FALSE(http::SyntaxChecker::IsValidVersionName(version));
}

TEST(VersionTests, 2_Test)
{
    std::string version = " ";
    EXPECT_FALSE(http::SyntaxChecker::IsValidVersionName(version));
}

TEST(VersionTests, 3_Test)
{
    std::string version = "HTTP/1.0";
    EXPECT_TRUE(http::SyntaxChecker::IsValidVersionName(version));
}

TEST(VersionTests, 4_Test)
{
    std::string version = "HTTP/1.1";
    EXPECT_TRUE(http::SyntaxChecker::IsValidVersionName(version));
}

TEST(VersionTests, 5_Test)
{
    std::string version = "\rHTTP/1.1";
    EXPECT_FALSE(http::SyntaxChecker::IsValidVersionName(version));
}

TEST(VersionTests, 6_Test)
{
    std::string version = "HTTP/1.1\n";
    EXPECT_FALSE(http::SyntaxChecker::IsValidVersionName(version));
}

TEST(VersionTests, 7_Test)
{
    std::string version = "HTTP/1.1 ";
    EXPECT_FALSE(http::SyntaxChecker::IsValidVersionName(version));
}

TEST(HeaderValueTests, 1_Test)
{
    std::string header_value = "";
    EXPECT_TRUE(http::SyntaxChecker::IsValidHeaderValueName(header_value));
}

TEST(HeaderValueTests, 2_Test)
{
    std::string header_value = " ";
    EXPECT_FALSE(http::SyntaxChecker::IsValidHeaderValueName(header_value));
}

TEST(HeaderValueTests, 3_Test)
{
    std::string header_value = "a";
    EXPECT_TRUE(http::SyntaxChecker::IsValidHeaderValueName(header_value));
}

TEST(HeaderValueTests, 4_Test)
{
    std::string header_value = "1";
    EXPECT_TRUE(http::SyntaxChecker::IsValidHeaderValueName(header_value));
}

TEST(HeaderValueTests, 5_Test)
{
    std::string header_value = "ä";
    EXPECT_TRUE(http::SyntaxChecker::IsValidHeaderValueName(header_value));
}

TEST(HeaderValueTests, 6_Test)
{
    std::string header_value = "=";
    EXPECT_TRUE(http::SyntaxChecker::IsValidHeaderValueName(header_value));
}

TEST(HeaderValueTests, 7_Test)
{
    std::string header_value = "hello world";
    EXPECT_TRUE(http::SyntaxChecker::IsValidHeaderValueName(header_value));
}

TEST(HeaderValueTests, 8_Test)
{
    std::string header_value = "\"#a0+-[ {@;,\t...";
    EXPECT_TRUE(http::SyntaxChecker::IsValidHeaderValueName(header_value));
}