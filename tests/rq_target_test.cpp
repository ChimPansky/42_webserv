#include <iostream>

#include "../src/http/RqTarget.h"
#include "../src/http/http.h"
#include "gtest/gtest.h"

TEST(BasicTests, 1_Test)
{
    http::RqTarget RqTarget = http::RqTarget("/path/to/file?a=20&b=30#fragment");
    EXPECT_EQ(
        http::RqTarget(RqTarget.scheme(), RqTarget.user_info(), RqTarget.host(), RqTarget.port(),
                       RqTarget.path(), RqTarget.query(), RqTarget.fragment()),
        RqTarget);
    EXPECT_EQ(RqTarget.ToStr(), "/path/to/file?a=20&b=30#fragment");
    EXPECT_EQ(RqTarget.scheme(), "");
    EXPECT_EQ(RqTarget.user_info(), "");
    EXPECT_EQ(RqTarget.host(), "");
    EXPECT_EQ(RqTarget.port(), "");
    EXPECT_EQ(RqTarget.path(), "/path/to/file");
    EXPECT_EQ(RqTarget.query(), "a=20&b=30");
    EXPECT_EQ(RqTarget.fragment(), "fragment");
    EXPECT_FALSE(RqTarget.HasScheme());
    EXPECT_FALSE(RqTarget.HasUserInfo());
    EXPECT_FALSE(RqTarget.HasHost());
    EXPECT_FALSE(RqTarget.HasPort());
    EXPECT_TRUE(RqTarget.HasPath());
    EXPECT_TRUE(RqTarget.HasQuery());
    EXPECT_TRUE(RqTarget.HasFragment());
    EXPECT_FALSE(RqTarget.validity_state() & http::RqTarget::RQ_TARGET_BAD_SCHEME);
    EXPECT_FALSE(RqTarget.validity_state() & http::RqTarget::RQ_TARGET_HAS_USER_INFO);
    EXPECT_FALSE(RqTarget.validity_state() & http::RqTarget::RQ_TARGET_BAD_HOST);
    EXPECT_FALSE(RqTarget.validity_state() & http::RqTarget::RQ_TARGET_BAD_PORT);
    EXPECT_FALSE(RqTarget.validity_state() & http::RqTarget::RQ_TARGET_BAD_PATH);
    EXPECT_FALSE(RqTarget.validity_state() & http::RqTarget::RQ_TARGET_BAD_QUERY);
    EXPECT_TRUE(RqTarget.validity_state() & http::RqTarget::RQ_TARGET_HAS_FRAGMENT);
    EXPECT_FALSE(RqTarget.Good());
}

TEST(BasicTests, 2_Test)
{
    http::RqTarget RqTarget = http::RqTarget("/path/to/file?a=20&b=30#fragment");
    EXPECT_EQ("/path/to/file?a=20&b=30#fragment", RqTarget.ToStr());
    EXPECT_EQ("", RqTarget.scheme());
    EXPECT_EQ("", RqTarget.user_info());
    EXPECT_EQ("", RqTarget.host());
    EXPECT_EQ("", RqTarget.port());
    EXPECT_EQ("/path/to/file", RqTarget.path());
    EXPECT_EQ("a=20&b=30", RqTarget.query());
    EXPECT_EQ("fragment", RqTarget.fragment());
    EXPECT_FALSE(RqTarget.HasScheme());
    EXPECT_FALSE(RqTarget.HasUserInfo());
    EXPECT_FALSE(RqTarget.HasHost());
    EXPECT_FALSE(RqTarget.HasPort());
    EXPECT_TRUE(RqTarget.HasPath());
    EXPECT_TRUE(RqTarget.HasQuery());
    EXPECT_TRUE(RqTarget.HasFragment());
    EXPECT_EQ(RqTarget.validity_state(), http::RqTarget::RQ_TARGET_HAS_FRAGMENT);
    EXPECT_FALSE(RqTarget.Good());
}

TEST(BasicTests, 3_Test)
{
    http::RqTarget RqTarget = http::RqTarget("http://host:1234/path/to/file?somequery");
    EXPECT_EQ("http://host:1234/path/to/file?somequery", RqTarget.ToStr());
    EXPECT_EQ("http", RqTarget.scheme());
    EXPECT_EQ("", RqTarget.user_info());
    EXPECT_EQ("host", RqTarget.host());
    EXPECT_EQ("1234", RqTarget.port());
    EXPECT_EQ("/path/to/file", RqTarget.path());
    EXPECT_EQ("somequery", RqTarget.query());
    EXPECT_EQ("", RqTarget.fragment());
    EXPECT_TRUE(RqTarget.HasScheme());
    EXPECT_FALSE(RqTarget.HasUserInfo());
    EXPECT_TRUE(RqTarget.HasHost());
    EXPECT_TRUE(RqTarget.HasPort());
    EXPECT_TRUE(RqTarget.HasPath());
    EXPECT_TRUE(RqTarget.HasQuery());
    EXPECT_FALSE(RqTarget.HasFragment());
    EXPECT_TRUE(RqTarget.Good());
}

TEST(BasicTests, 4_Test)
{
    http::RqTarget RqTarget = http::RqTarget("http://user:password@www.test.com/?");
    EXPECT_EQ("http://user:password@www.test.com/?", RqTarget.ToStr());
    EXPECT_EQ("http", RqTarget.scheme());
    EXPECT_EQ("user:password", RqTarget.user_info());
    EXPECT_EQ("www.test.com", RqTarget.host());
    EXPECT_EQ("", RqTarget.port());
    EXPECT_EQ("/", RqTarget.path());
    EXPECT_EQ("", RqTarget.query());
    EXPECT_EQ("", RqTarget.fragment());
    EXPECT_TRUE(RqTarget.HasScheme());
    EXPECT_TRUE(RqTarget.HasUserInfo());
    EXPECT_TRUE(RqTarget.HasHost());
    EXPECT_FALSE(RqTarget.HasPort());
    EXPECT_TRUE(RqTarget.HasPath());
    EXPECT_TRUE(RqTarget.HasQuery());
    EXPECT_FALSE(RqTarget.HasFragment());
    EXPECT_EQ(RqTarget.validity_state(), http::RqTarget::RQ_TARGET_HAS_USER_INFO);
    EXPECT_FALSE(RqTarget.Good());
}

TEST(BasicTests, 5_Test)
{
    http::RqTarget RqTarget = http::RqTarget("");
    EXPECT_TRUE(RqTarget.validity_state() & http::RqTarget::RQ_TARGET_BAD_PATH);
    EXPECT_FALSE(RqTarget.Good());
}

TEST(BasicTests, 6_Test)
{
    http::RqTarget RqTarget = http::RqTarget(".");
    EXPECT_TRUE(RqTarget.validity_state() & http::RqTarget::RQ_TARGET_BAD_PATH);
    EXPECT_FALSE(RqTarget.Good());
}

TEST(BasicTests, 7_Test)
{
    http::RqTarget RqTarget = http::RqTarget("/");
    EXPECT_EQ(RqTarget.scheme(), "");
    EXPECT_EQ(RqTarget.user_info(), "");
    EXPECT_EQ(RqTarget.host(), "");
    EXPECT_EQ(RqTarget.port(), "");
    EXPECT_EQ(RqTarget.path(), "/");
    EXPECT_EQ(RqTarget.query(), "");
    EXPECT_EQ(RqTarget.fragment(), "");
    EXPECT_EQ(RqTarget.HasScheme(), false);
    EXPECT_EQ(RqTarget.HasUserInfo(), false);
    EXPECT_EQ(RqTarget.HasHost(), false);
    EXPECT_EQ(RqTarget.HasPort(), false);
    EXPECT_EQ(RqTarget.HasPath(), true);
    EXPECT_EQ(RqTarget.HasQuery(), false);
    EXPECT_EQ(RqTarget.HasFragment(), false);
    EXPECT_TRUE(RqTarget.Good());
}

TEST(BasicTests, 8_Test)
{
    http::RqTarget RqTarget = http::RqTarget("?");
    EXPECT_TRUE(RqTarget.validity_state() & http::RqTarget::RQ_TARGET_BAD_PATH);
    EXPECT_FALSE(RqTarget.Good());
}

TEST(BasicTests, 9_Test)
{
    http::RqTarget RqTarget = http::RqTarget("#");
    EXPECT_TRUE(RqTarget.validity_state() & http::RqTarget::RQ_TARGET_BAD_PATH);
    EXPECT_FALSE(RqTarget.Good());
}

TEST(BasicTests, 10_Test)
{
    http::RqTarget RqTarget = http::RqTarget(" ");
    EXPECT_TRUE(RqTarget.validity_state() & http::RqTarget::RQ_TARGET_BAD_PATH);
    EXPECT_FALSE(RqTarget.Good());
}

TEST(BasicTests, 11_Test)
{
    http::RqTarget RqTarget = http::RqTarget("abc");
    EXPECT_TRUE(RqTarget.validity_state() & http::RqTarget::RQ_TARGET_BAD_PATH);
    EXPECT_FALSE(RqTarget.Good());
}

TEST(BasicTests, 12_Test)
{
    http::RqTarget RqTarget = http::RqTarget(
        "/llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiiii/"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiiii/"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiiii/"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiii//"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiiii/"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiiii/"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiiii/"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiii//"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiiii/"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiiii/"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiiii/"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiii//"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiiii/"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiiii/"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiiii/"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiii//"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiiii/"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiiii/"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiiii/"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiii//"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiiii/"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiiii/"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiiii/"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiii//"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiiii/"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiiii/"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiiii/"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiii//"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiiii/"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiiii/"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiiii/"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiii//"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiiii/"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiiii/"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiiii/"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiii//"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiiii/"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiiii/"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiiii/"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiii//"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiiii/"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiiii/"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiiii/"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiii//"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiiii/"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiiii/"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiiii/"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiii//"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiiii/"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiiii/"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiiii/"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiii//"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiiii/"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiiii/"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiiii/"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiii//"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiiii/"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiiii/"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiiii/"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiii//"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiiii/"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiiii/"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiiii/"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiii//"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiiii/"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiiii/"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiiii/"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiii//"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiiii/"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiiii/"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiiii/"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiii//"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiiii/"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiiii/"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiiii/"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiii//"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiiii/"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiiii/"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiiii/"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiii//"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiiii/"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiiii/"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiiii/"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiii//"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiiii/"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiiii/"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiiii/"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiii//"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiiii/"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiiii/"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiiii/"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiii//"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiiii/"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiiii/"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiiii/"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiii//"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiiii/"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiiii/"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiiii/"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiii//"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiiii/"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiiii/"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiiii/"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiii//"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiiii/"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiiii/"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiiii/"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiii//"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiiii/"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiiii/"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiiii/"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiii//"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiiii/"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiiii/"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiiii/"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiii//"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiiii/"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiiii/"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiiii/"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiii//"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiiii/"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiiii/"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiiii/"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiii//"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiiii/"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiiii/"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiiii/"
        "llooooooonnnnnnnnnngggggggggggggguuuuuuuuuuuurrrrrrrriiiiiiiiii/");
    if (RQ_TARGET_LEN_LIMIT > 256) {
        EXPECT_TRUE(RqTarget.validity_state() & http::RqTarget::RQ_TARGET_TOO_LONG);
    } else {
        EXPECT_FALSE(RqTarget.validity_state() & http::RqTarget::RQ_TARGET_TOO_LONG);
        EXPECT_TRUE(RqTarget.Good());
    }
}

TEST(SchemeTests, 1_Test)
{
    http::RqTarget RqTarget = http::RqTarget("htp://");
    EXPECT_TRUE(RqTarget.validity_state() & http::RqTarget::RQ_TARGET_BAD_SCHEME);
    EXPECT_FALSE(RqTarget.Good());
}

TEST(SchemeTests, 2_Test)
{
    http::RqTarget RqTarget = http::RqTarget("1://");
    EXPECT_TRUE(RqTarget.validity_state() & http::RqTarget::RQ_TARGET_BAD_SCHEME);
    EXPECT_FALSE(RqTarget.Good());
}

TEST(SchemeTests, 3_Test)
{
    http::RqTarget RqTarget = http::RqTarget("://");
    EXPECT_TRUE(RqTarget.validity_state() & http::RqTarget::RQ_TARGET_BAD_SCHEME);
    EXPECT_FALSE(RqTarget.Good());
}

TEST(SchemeTests, 4_Test)
{
    http::RqTarget RqTarget = http::RqTarget("http://www.example.com/");
    EXPECT_EQ(RqTarget.scheme(), "http");
    EXPECT_EQ(RqTarget.user_info(), "");
    EXPECT_EQ(RqTarget.host(), "www.example.com");
    EXPECT_EQ(RqTarget.port(), "");
    EXPECT_EQ(RqTarget.path(), "/");
    EXPECT_EQ(RqTarget.query(), "");
    EXPECT_EQ(RqTarget.fragment(), "");
    EXPECT_EQ(RqTarget.HasScheme(), true);
    EXPECT_EQ(RqTarget.HasUserInfo(), false);
    EXPECT_EQ(RqTarget.HasHost(), true);
    EXPECT_EQ(RqTarget.HasPort(), false);
    EXPECT_EQ(RqTarget.HasPath(), true);
    EXPECT_EQ(RqTarget.HasQuery(), false);
    EXPECT_EQ(RqTarget.HasFragment(), false);
    EXPECT_TRUE(RqTarget.Good());
}


TEST(HostTests, 1_Test)
{
    http::RqTarget RqTarget = http::RqTarget("http://:1234");
    EXPECT_TRUE(RqTarget.validity_state() & http::RqTarget::RQ_TARGET_BAD_HOST);
    EXPECT_FALSE(RqTarget.Good());
}

TEST(HostTests, 2_Test)
{
    http::RqTarget RqTarget = http::RqTarget("http://host:");
    EXPECT_TRUE(RqTarget.validity_state() & http::RqTarget::RQ_TARGET_BAD_PORT);
    EXPECT_FALSE(RqTarget.Good());
}

TEST(HostTests, 3_Test)
{
    http::RqTarget RqTarget = http::RqTarget("http://host:port");
    EXPECT_TRUE(RqTarget.validity_state() & http::RqTarget::RQ_TARGET_BAD_PORT);
    EXPECT_FALSE(RqTarget.Good());
}

TEST(HostTests, 4_Test)
{
    http::RqTarget RqTarget = http::RqTarget("http://*/");
    EXPECT_TRUE(RqTarget.validity_state() & http::RqTarget::RQ_TARGET_BAD_HOST);
    EXPECT_EQ(RqTarget.path(), "/");
    EXPECT_FALSE(RqTarget.Good());
}

TEST(HostTests, 5_Test)
{
    http::RqTarget RqTarget = http::RqTarget("http://abc=def/path");
    EXPECT_TRUE(RqTarget.validity_state() & http::RqTarget::RQ_TARGET_BAD_HOST);
    EXPECT_EQ(RqTarget.path(), "/path");
    EXPECT_FALSE(RqTarget.Good());
}

TEST(HostTests, 6_Test)
{
    http::RqTarget RqTarget = http::RqTarget("http://äöü/path/to/file?query");
    EXPECT_TRUE(RqTarget.validity_state() & http::RqTarget::RQ_TARGET_BAD_HOST);
    EXPECT_EQ(RqTarget.path(), "/path/to/file");
    EXPECT_EQ(RqTarget.query(), "query");
    EXPECT_FALSE(RqTarget.Good());
}

TEST(HostTests, 7_Test)
{
    http::RqTarget RqTarget = http::RqTarget("http://www.abc]def.com/");
    EXPECT_TRUE(RqTarget.validity_state() & http::RqTarget::RQ_TARGET_BAD_HOST);
    EXPECT_EQ(RqTarget.path(), "/");
    EXPECT_FALSE(RqTarget.Good());
}

TEST(HostTests, 8_Test)
{
    http::RqTarget RqTarget = http::RqTarget("http://www.crazy!website.com");
    EXPECT_TRUE(RqTarget.validity_state() & http::RqTarget::RQ_TARGET_BAD_HOST);
    EXPECT_TRUE(RqTarget.validity_state() & http::RqTarget::RQ_TARGET_BAD_PATH);
    EXPECT_FALSE(RqTarget.Good());
}

TEST(PortTests, 1_Test)
{
    http::RqTarget RqTarget = http::RqTarget("http://host:port");
    EXPECT_TRUE(RqTarget.validity_state() & http::RqTarget::RQ_TARGET_BAD_PORT);
    EXPECT_FALSE(RqTarget.Good());
}

TEST(PortTests, 2_Test)
{
    http::RqTarget RqTarget = http::RqTarget("http://host:-1");
    EXPECT_TRUE(RqTarget.validity_state() & http::RqTarget::RQ_TARGET_BAD_PORT);
    EXPECT_FALSE(RqTarget.Good());
}

TEST(PortTests, 3_Test)
{
    http::RqTarget RqTarget = http::RqTarget("http://host:65536");
    EXPECT_TRUE(RqTarget.validity_state() & http::RqTarget::RQ_TARGET_BAD_PORT);
    EXPECT_FALSE(RqTarget.Good());
}

TEST(PortTests, 4_Test)
{
    http::RqTarget RqTarget = http::RqTarget("http://host:65535/");
    EXPECT_EQ(RqTarget.port(), "65535");
    EXPECT_TRUE(RqTarget.HasPort());
    EXPECT_TRUE(RqTarget.Good());
}

TEST(PortTests, 5_Test)
{
    http::RqTarget RqTarget = http::RqTarget("http://host:80/");
    EXPECT_EQ(RqTarget.port(), "");
    EXPECT_FALSE(RqTarget.HasPort());
    EXPECT_TRUE(RqTarget.Good());
}

TEST(PathTests, 1_Test)
{
    http::RqTarget RqTarget = http::RqTarget("//");
    EXPECT_TRUE(RqTarget.validity_state() & http::RqTarget::RQ_TARGET_BAD_PATH);
    EXPECT_FALSE(RqTarget.Good());
}

TEST(PathTests, 2_Test)
{
    http::RqTarget RqTarget = http::RqTarget("");
    EXPECT_TRUE(RqTarget.validity_state() & http::RqTarget::RQ_TARGET_BAD_PATH);
    EXPECT_FALSE(RqTarget.Good());
}

TEST(PathTests, 3_Test)
{
    http::RqTarget RqTarget = http::RqTarget("/ä");
    EXPECT_TRUE(RqTarget.validity_state() & http::RqTarget::RQ_TARGET_BAD_PATH);
}

TEST(PathTests, 4_Test)
{
    http::RqTarget RqTarget = http::RqTarget("/[bad]path");
    EXPECT_TRUE(RqTarget.validity_state() & http::RqTarget::RQ_TARGET_BAD_PATH);
}

TEST(PathTests, 5_Test)
{
    http::RqTarget RqTarget = http::RqTarget("/badpath[]");
    EXPECT_TRUE(RqTarget.validity_state() & http::RqTarget::RQ_TARGET_BAD_PATH);
}

TEST(PathTests, 6_Test)
{
    http::RqTarget RqTarget = http::RqTarget("/{badpath}/some_more");
    EXPECT_TRUE(RqTarget.validity_state() & http::RqTarget::RQ_TARGET_BAD_PATH);
}

TEST(PathTests, 7_Test)
{
    http::RqTarget RqTarget = http::RqTarget("/bad path");
    EXPECT_TRUE(RqTarget.validity_state() & http::RqTarget::RQ_TARGET_BAD_PATH);
}

TEST(PathTests, 8_Test)
{
    http::RqTarget RqTarget = http::RqTarget("/ok+path");
    EXPECT_EQ(RqTarget.path(), "/ok+path");
    EXPECT_TRUE(RqTarget.Good());
}

TEST(PathTests, 9_Test)
{
    http::RqTarget RqTarget = http::RqTarget("/ok%20path");
    EXPECT_EQ(RqTarget.path(), "/ok%20path");
    EXPECT_TRUE(RqTarget.Good());
}

TEST(PathTests, 10_Test)
{
    http::RqTarget RqTarget = http::RqTarget("/this_is_allowed/~");
    EXPECT_EQ(RqTarget.path(), "/this_is_allowed/~");
    EXPECT_TRUE(RqTarget.Good());
}

TEST(PathTests, 11_Test)
{
    http::RqTarget RqTarget = http::RqTarget("/this+is+fine/9.9/.abc");
    EXPECT_EQ(RqTarget.path(), "/this+is+fine/9.9/.abc");
    EXPECT_TRUE(RqTarget.Good());
}

TEST(PathTests, 12_Test)
{
    http::RqTarget RqTarget = http::RqTarget("/sub_delims_are_fine/!$&'()*+,;");
    EXPECT_EQ(RqTarget.path(), "/sub_delims_are_fine/!$&'()*+,;");
    EXPECT_TRUE(RqTarget.Good());
}

TEST(PathTests, 13_Test)
{
    http::RqTarget RqTarget = http::RqTarget("/unreserved_chars_are_fine/abcABC123-_.~");
    EXPECT_EQ(RqTarget.path(), "/unreserved_chars_are_fine/abcABC123-_.~");
    EXPECT_TRUE(RqTarget.Good());
}

TEST(PathDecoding, 1_Test)
{
    http::RqTarget RqTarget = http::RqTarget("/invalid_encoding%2");
    EXPECT_TRUE(RqTarget.validity_state() & http::RqTarget::RQ_TARGET_BAD_PATH);
}

TEST(PathDecoding, 2_Test)
{
    http::RqTarget RqTarget = http::RqTarget("/invalid_encoding%2G");
    EXPECT_TRUE(RqTarget.validity_state() & http::RqTarget::RQ_TARGET_BAD_PATH);
}

TEST(PathDecoding, 3_Test)
{
    http::RqTarget RqTarget = http::RqTarget("/invalid_encoding%Z");
    EXPECT_TRUE(RqTarget.validity_state() & http::RqTarget::RQ_TARGET_BAD_PATH);
}

TEST(PathDecoding, 4_Test)
{
    http::RqTarget RqTarget = http::RqTarget("/invalid_encoding%9xabc");
    EXPECT_TRUE(RqTarget.validity_state() & http::RqTarget::RQ_TARGET_BAD_PATH);
}

TEST(PathDecoding, 5_Test)
{
    http::RqTarget RqTarget = http::RqTarget("/%5A%5a%5a");
    EXPECT_EQ(RqTarget.path(), "/ZZZ");
    EXPECT_TRUE(RqTarget.Good());
}

TEST(PathDecoding, 6_Test)
{
    http::RqTarget RqTarget = http::RqTarget("/abc%2Fdef");
    EXPECT_EQ(RqTarget.path(), "/abc%2Fdef");
    EXPECT_TRUE(RqTarget.Good());
}

TEST(PathDecoding, 7_Test)
{
    http::RqTarget RqTarget = http::RqTarget("/abc%2fdef");
    EXPECT_EQ(RqTarget.path(), "/abc%2Fdef");
    EXPECT_TRUE(RqTarget.Good());
}

TEST(PathDecoding, 8_Test)
{
    http::RqTarget RqTarget = http::RqTarget("/abc%2fdef%2Fghi");
    EXPECT_EQ(RqTarget.path(), "/abc%2Fdef%2Fghi");
    EXPECT_TRUE(RqTarget.Good());
}

TEST(PathDecoding, 9_Test)
{
    http::RqTarget RqTarget = http::RqTarget("/encoded/dots/will/be/decoded%2E%2E%2E");
    EXPECT_EQ(RqTarget.path(), "/encoded/dots/will/be/decoded...");
    EXPECT_TRUE(RqTarget.Good());
}

TEST(PathDotSegments, 1_Test)
{
    http::RqTarget RqTarget = http::RqTarget("/.");
    EXPECT_EQ(RqTarget.path(), "/");
    EXPECT_TRUE(RqTarget.Good());
}

TEST(PathDotSegments, 2_Test)
{
    http::RqTarget RqTarget = http::RqTarget("/./");
    EXPECT_EQ(RqTarget.path(), "/");
    EXPECT_TRUE(RqTarget.Good());
}

TEST(PathDotSegments, 3_Test)
{
    http::RqTarget RqTarget = http::RqTarget("/..");
    EXPECT_TRUE(RqTarget.validity_state() & http::RqTarget::RQ_TARGET_BAD_PATH);
}

TEST(PathDotSegments, 4_Test)
{
    http::RqTarget RqTarget = http::RqTarget("/../");
    EXPECT_TRUE(RqTarget.validity_state() & http::RqTarget::RQ_TARGET_BAD_PATH);
}

TEST(PathDotSegments, 5_Test)
{
    http::RqTarget RqTarget = http::RqTarget("/a.b/");
    EXPECT_EQ(RqTarget.path(), "/a.b/");
    EXPECT_TRUE(RqTarget.Good());
}

TEST(PathDotSegments, 6_Test)
{
    http::RqTarget RqTarget = http::RqTarget("/a..b/");
    EXPECT_EQ(RqTarget.path(), "/a..b/");
    EXPECT_TRUE(RqTarget.Good());
}

TEST(PathDotSegments, 7_Test)
{
    http::RqTarget RqTarget = http::RqTarget("/a...b/");
    EXPECT_EQ(RqTarget.path(), "/a...b/");
    EXPECT_TRUE(RqTarget.Good());
}

TEST(PathDotSegments, 8_Test)
{
    http::RqTarget RqTarget = http::RqTarget("/some_path/../");
    EXPECT_EQ(RqTarget.path(), "/");
    EXPECT_TRUE(RqTarget.Good());
}

TEST(PathDotSegments, 9_Test)
{
    http::RqTarget RqTarget = http::RqTarget("/some_path/../../");
    EXPECT_TRUE(RqTarget.validity_state() & http::RqTarget::RQ_TARGET_BAD_PATH);
}

TEST(PathDotSegments, 10_Test)
{
    http::RqTarget RqTarget = http::RqTarget("/some_path/../.../");
    EXPECT_EQ(RqTarget.path(), "/.../");
    EXPECT_TRUE(RqTarget.Good());
}

TEST(PathDotSegments, 11_Test)
{
    http::RqTarget RqTarget = http::RqTarget("/abc/./def/../ghi");
    EXPECT_EQ(RqTarget.path(), "/abc/ghi");
    EXPECT_TRUE(RqTarget.Good());
}

TEST(PathDotSegments, 12_Test)
{
    http::RqTarget RqTarget = http::RqTarget("/a/b/c/./../../g");
    EXPECT_EQ(RqTarget.path(), "/a/g");
    EXPECT_TRUE(RqTarget.Good());
}

TEST(PathDotSegments, 13_Test)
{
    http::RqTarget RqTarget = http::RqTarget("/a/b/./c/../d/.");
    EXPECT_EQ(RqTarget.path(), "/a/b/d/");
    EXPECT_TRUE(RqTarget.Good());
}

TEST(PathDotSegments, 14_Test)
{
    http::RqTarget RqTarget = http::RqTarget("/a/b/c/./../../g/../h/./i/../j");
    EXPECT_EQ(RqTarget.path(), "/a/h/j");
    EXPECT_TRUE(RqTarget.Good());
}

TEST(PathDotSegments, 15_Test)
{
    http::RqTarget RqTarget = http::RqTarget("/a/./b/c/../../d/e/../f/./g/../h");
    EXPECT_EQ(RqTarget.path(), "/a/d/f/h");
    EXPECT_TRUE(RqTarget.Good());
}

TEST(PathDotSegments, 16_Test)
{
    http::RqTarget RqTarget = http::RqTarget("/a/./b/../c/././d/../../e/../f/..");
    EXPECT_EQ(RqTarget.path(), "/a/");
    EXPECT_TRUE(RqTarget.Good());
}

TEST(PathCollapseSlashes, 1_Test)
{
    http::RqTarget RqTarget = http::RqTarget("/a//b/c//d//e");
    EXPECT_EQ(RqTarget.path(), "/a/b/c/d/e");
    EXPECT_TRUE(RqTarget.Good());
}

TEST(PathCollapseSlashes, 2_Test)
{
    http::RqTarget RqTarget = http::RqTarget("/a////////////");
    EXPECT_EQ(RqTarget.path(), "/a/");
    EXPECT_TRUE(RqTarget.Good());
}

TEST(PathCollapseSlashes, 3_Test)
{
    http::RqTarget RqTarget = http::RqTarget("//");
    EXPECT_TRUE(RqTarget.validity_state() & http::RqTarget::RQ_TARGET_BAD_PATH);
}

TEST(PathCollapseSlashes, 4_Test)
{
    http::RqTarget RqTarget = http::RqTarget("/some_path/%2F%2f/b///c//////");
    EXPECT_EQ(RqTarget.path(), "/some_path/%2F%2F/b/c/");
    EXPECT_TRUE(RqTarget.Good());
}

TEST(QueryTests, 1_Test)
{
    http::RqTarget RqTarget = http::RqTarget("/?");
    EXPECT_TRUE(RqTarget.HasQuery());
    EXPECT_EQ(RqTarget.query(), "");
    EXPECT_TRUE(RqTarget.Good());
}

TEST(QueryTests, 2_Test)
{
    http::RqTarget RqTarget = http::RqTarget("/?a");
    EXPECT_TRUE(RqTarget.HasQuery());
    EXPECT_EQ(RqTarget.query(), "a");
    EXPECT_TRUE(RqTarget.Good());
}

TEST(QueryTests, 3_Test)
{
    http::RqTarget RqTarget = http::RqTarget("/? ");
    EXPECT_TRUE(RqTarget.HasQuery());
    EXPECT_TRUE(RqTarget.validity_state() & http::RqTarget::RQ_TARGET_BAD_QUERY);
}

TEST(QueryTests, 4_Test)
{
    http::RqTarget RqTarget = http::RqTarget("/?:@/?");
    EXPECT_TRUE(RqTarget.HasQuery());
    EXPECT_EQ(RqTarget.query(), ":@/?");
    EXPECT_TRUE(RqTarget.Good());
}

TEST(QueryTests, 5_Test)
{
    http::RqTarget RqTarget = http::RqTarget("/?abcABC123-_.~");
    EXPECT_TRUE(RqTarget.HasQuery());
    EXPECT_EQ(RqTarget.query(), "abcABC123-_.~");
    EXPECT_TRUE(RqTarget.Good());
}

TEST(QueryTests, 6_Test)
{
    http::RqTarget RqTarget = http::RqTarget("/?a=%2F%2f");
    EXPECT_TRUE(RqTarget.HasQuery());
    EXPECT_EQ(RqTarget.query(), "a=%2F%2F");
    EXPECT_TRUE(RqTarget.Good());
}

TEST(QueryTests, 7_Test)
{
    http::RqTarget RqTarget = http::RqTarget("/?search=wildcard%2a");
    EXPECT_TRUE(RqTarget.HasQuery());
    EXPECT_EQ(RqTarget.query(), "search=wildcard%2A");
    EXPECT_TRUE(RqTarget.Good());
}

TEST(QueryTests, 8_Test)
{
    http::RqTarget RqTarget = http::RqTarget(
        "/folder?param1=value1;param2=https%3A%2F%2Fwww.example.com%3a443%2Fpath%2Fto%2Ffile%"
        "3Fquery%3Dvalue");
    EXPECT_TRUE(RqTarget.HasQuery());
    EXPECT_EQ(RqTarget.query(),
              "param1=value1;param2=https%3A%2F%2Fwww.example.com%3A443%2Fpath%2Fto%2Ffile%3Fquery%"
              "3Dvalue");
    EXPECT_TRUE(RqTarget.Good());
}

TEST(QueryTests, 9_Test)
{
    http::RqTarget RqTarget = http::RqTarget("/?path=%2ffolder%2f..%2fother%20folder");
    EXPECT_TRUE(RqTarget.HasQuery());
    EXPECT_EQ(RqTarget.query(), "path=%2Ffolder%2F..%2Fother%20folder");
    EXPECT_TRUE(RqTarget.Good());
}

TEST(GoCrazy, 1_Test)
{
    http::RqTarget RqTarget = http::RqTarget("http://eXAMPLE/a/./b/../b/%63/%7bfoo%7d");
    EXPECT_EQ(RqTarget.scheme(), "http");
    EXPECT_EQ(RqTarget.user_info(), "");
    EXPECT_EQ(RqTarget.host(), "example");
    EXPECT_EQ(RqTarget.port(), "");
    EXPECT_EQ(RqTarget.path(), "/a/b/c/%7Bfoo%7D");
    EXPECT_EQ(RqTarget.query(), "");
    EXPECT_EQ(RqTarget.fragment(), "");
    EXPECT_EQ(RqTarget.HasScheme(), true);
    EXPECT_EQ(RqTarget.HasUserInfo(), false);
    EXPECT_EQ(RqTarget.HasHost(), true);
    EXPECT_EQ(RqTarget.HasPort(), false);
    EXPECT_EQ(RqTarget.HasPath(), true);
    EXPECT_EQ(RqTarget.HasQuery(), false);
    EXPECT_EQ(RqTarget.HasFragment(), false);
    EXPECT_TRUE(RqTarget.Good());
}


// malicious (directory-traversal) encoded uri in query...
TEST(GoCrazy, 2_Test)
{
    http::RqTarget RqTarget = http::RqTarget(
        "http://eXAMPLE:5555/hello%20world////./dot..name/b/%2e%2E/"
        "?search=%2e%2E&link=http%3a%2F%2Fwww.example.com%2Fupload%2f%3Ffile%3d%2E%2E%2F%2e%2E%2F%"
        "2E%2e%2Fetc%2Fpasswd%26user%3Dadmin%26pass%3dadmin");
    EXPECT_EQ(RqTarget.scheme(), "http");
    EXPECT_EQ(RqTarget.user_info(), "");
    EXPECT_EQ(RqTarget.host(), "example");
    EXPECT_EQ(RqTarget.port(), "5555");
    EXPECT_EQ(RqTarget.path(), "/hello%20world/dot..name/");
    EXPECT_EQ(RqTarget.query(),
              "search=..&link=http%3A%2F%2Fwww.example.com%2Fupload%2F%3Ffile%3D..%2F..%2F..%2Fetc%"
              "2Fpasswd%26user%3Dadmin%26pass%3Dadmin");
    EXPECT_EQ(RqTarget.fragment(), "");
    EXPECT_EQ(RqTarget.HasScheme(), true);
    EXPECT_EQ(RqTarget.HasUserInfo(), false);
    EXPECT_EQ(RqTarget.HasHost(), true);
    EXPECT_EQ(RqTarget.HasPort(), true);
    EXPECT_EQ(RqTarget.HasPath(), true);
    EXPECT_EQ(RqTarget.HasQuery(), true);
    EXPECT_EQ(RqTarget.HasFragment(), false);
    EXPECT_TRUE(RqTarget.Good());
}
