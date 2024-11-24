#include "gtest/gtest.h"
#include "../src/http/RqTarget.h"
#include <iostream>

TEST(BasicTests, 1_Test) {
    http::RqTarget RqTarget = http::RqTarget("/path/to/file?a=20&b=30#fragment");
    EXPECT_EQ(http::RqTarget(RqTarget.scheme(), RqTarget.user_info(), RqTarget.host(), RqTarget.port(), RqTarget.path(), RqTarget.query(), RqTarget.fragment()), RqTarget);
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
    EXPECT_EQ(RqTarget.Good(), false);

}

TEST(BasicTests, 2_Test) {
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
    EXPECT_EQ(RqTarget.Good(), false);
}

TEST(BasicTests, 3_Test) {
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
    EXPECT_EQ(RqTarget.Good(), true);
}

TEST(BasicTests, 4_Test) {
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
    EXPECT_EQ(RqTarget.Good(), false);
}

TEST(BasicTests, 5_Test) {
    http::RqTarget RqTarget = http::RqTarget("");
    EXPECT_TRUE(RqTarget.validity_state() & http::RqTarget::RQ_TARGET_BAD_PATH);
    EXPECT_EQ(RqTarget.Good(), false);
}

TEST(BasicTests, 6_Test) {
    http::RqTarget RqTarget = http::RqTarget(".");
    EXPECT_TRUE(RqTarget.validity_state() & http::RqTarget::RQ_TARGET_BAD_PATH);
    EXPECT_EQ(RqTarget.Good(), false);
}

TEST(BasicTests, 7_Test) {
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
    EXPECT_EQ(RqTarget.Good(), true);
}

TEST(BasicTests, 8_Test) {
    http::RqTarget RqTarget = http::RqTarget("?");
    EXPECT_TRUE(RqTarget.validity_state() & http::RqTarget::RQ_TARGET_BAD_PATH);
    EXPECT_EQ(RqTarget.Good(), false);
}

TEST(BasicTests, 9_Test) {
    http::RqTarget RqTarget = http::RqTarget("#");
    EXPECT_TRUE(RqTarget.validity_state() & http::RqTarget::RQ_TARGET_BAD_PATH);
    EXPECT_EQ(RqTarget.Good(), false);
}

TEST(BasicTests, 10_Test) {
    http::RqTarget RqTarget = http::RqTarget(" ");
    EXPECT_TRUE(RqTarget.validity_state() & http::RqTarget::RQ_TARGET_BAD_PATH);
    EXPECT_EQ(RqTarget.Good(), false);
}

TEST(BasicTests, 11_Test) {
    http::RqTarget RqTarget = http::RqTarget("abc");
    EXPECT_TRUE(RqTarget.validity_state() & http::RqTarget::RQ_TARGET_BAD_PATH);
    EXPECT_EQ(RqTarget.Good(), false);
}

TEST(SchemeTests, 1_Test) {
    http::RqTarget RqTarget = http::RqTarget("htp://");
    EXPECT_TRUE(RqTarget.validity_state() & http::RqTarget::RQ_TARGET_BAD_SCHEME);
    EXPECT_EQ(RqTarget.Good(), false);
}

TEST(SchemeTests, 2_Test) {
    http::RqTarget RqTarget = http::RqTarget("1://");
    EXPECT_TRUE(RqTarget.validity_state() & http::RqTarget::RQ_TARGET_BAD_SCHEME);
    EXPECT_EQ(RqTarget.Good(), false);
}

TEST(SchemeTests, 3_Test) {
    http::RqTarget RqTarget = http::RqTarget("://");
    EXPECT_TRUE(RqTarget.validity_state() & http::RqTarget::RQ_TARGET_BAD_SCHEME);
    EXPECT_EQ(RqTarget.Good(), false);
}

TEST(SchemeTests, 4_Test) {
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
    EXPECT_EQ(RqTarget.Good(), true);
}


TEST(HostTests, 1_Test) {
    http::RqTarget RqTarget = http::RqTarget("http://:1234");
    EXPECT_TRUE(RqTarget.validity_state() & http::RqTarget::RQ_TARGET_BAD_HOST);
    EXPECT_EQ(RqTarget.Good(), false);
}

TEST(HostTests, 2_Test) {
    http::RqTarget RqTarget = http::RqTarget("http://host:");
    EXPECT_TRUE(RqTarget.validity_state() & http::RqTarget::RQ_TARGET_BAD_PORT);
    EXPECT_EQ(RqTarget.Good(), false);
}

TEST(HostTests, 3_Test) {
    http::RqTarget RqTarget = http::RqTarget("http://host:port");
    EXPECT_TRUE(RqTarget.validity_state() & http::RqTarget::RQ_TARGET_BAD_PORT);
    EXPECT_EQ(RqTarget.Good(), false);
}

TEST(HostTests, 4_Test) {
    http::RqTarget RqTarget = http::RqTarget("http://*/");
    EXPECT_TRUE(RqTarget.validity_state() & http::RqTarget::RQ_TARGET_BAD_HOST);
    EXPECT_EQ(RqTarget.path(), "/");
    EXPECT_EQ(RqTarget.Good(), false);
}

TEST(HostTests, 5_Test) {
    http::RqTarget RqTarget = http::RqTarget("http://abc=def/path");
    EXPECT_TRUE(RqTarget.validity_state() & http::RqTarget::RQ_TARGET_BAD_HOST);
    EXPECT_EQ(RqTarget.path(), "/path");
    EXPECT_EQ(RqTarget.Good(), false);
}

TEST(HostTests, 6_Test) {
    http::RqTarget RqTarget = http::RqTarget("http://äöü/path/to/file?query");
    EXPECT_TRUE(RqTarget.validity_state() & http::RqTarget::RQ_TARGET_BAD_HOST);
    EXPECT_EQ(RqTarget.path(), "/path/to/file");
    EXPECT_EQ(RqTarget.query(), "query");
    EXPECT_EQ(RqTarget.Good(), false);
}

TEST(HostTests, 7_Test) {
    http::RqTarget RqTarget = http::RqTarget("http://www.abc:def.com/");
    EXPECT_TRUE(RqTarget.validity_state() & http::RqTarget::RQ_TARGET_BAD_HOST);
    EXPECT_EQ(RqTarget.path(), "/");
    EXPECT_EQ(RqTarget.Good(), false);
}

TEST(HostTests, 8_Test) {
    http::RqTarget RqTarget = http::RqTarget("http://www.crazy!website.com");
    EXPECT_TRUE(RqTarget.validity_state() & http::RqTarget::RQ_TARGET_BAD_HOST);
    EXPECT_TRUE(RqTarget.validity_state() & http::RqTarget::RQ_TARGET_BAD_PATH);
    EXPECT_EQ(RqTarget.Good(), false);
}

TEST(PortTests, 1_Test) {
    http::RqTarget RqTarget = http::RqTarget("http://host:port");
    EXPECT_TRUE(RqTarget.validity_state() & http::RqTarget::RQ_TARGET_BAD_PORT);
    EXPECT_EQ(RqTarget.Good(), false);
}

TEST(PortTests, 2_Test) {
    http::RqTarget RqTarget = http::RqTarget("http://host:-1");
    EXPECT_TRUE(RqTarget.validity_state() & http::RqTarget::RQ_TARGET_BAD_PORT);
    EXPECT_EQ(RqTarget.Good(), false);
}

TEST(PortTests, 3_Test) {
    http::RqTarget RqTarget = http::RqTarget("http://host:65536");
    EXPECT_TRUE(RqTarget.validity_state() & http::RqTarget::RQ_TARGET_BAD_PORT);
    EXPECT_EQ(RqTarget.Good(), false);
}

TEST(PortTests, 4_Test) {
    http::RqTarget RqTarget = http::RqTarget("http://host:65535/");
    EXPECT_EQ(RqTarget.port(), "65535");
    EXPECT_TRUE(RqTarget.HasPort());
    EXPECT_EQ(RqTarget.Good(), true);
}

TEST(PortTests, 5_Test) {
    http::RqTarget RqTarget = http::RqTarget("http://host:80/");
    EXPECT_EQ(RqTarget.port(), "");
    EXPECT_FALSE(RqTarget.HasPort());
    EXPECT_EQ(RqTarget.Good(), true);
}

TEST(PathTests, 1_Test) {
    http::RqTarget RqTarget = http::RqTarget("//");
    EXPECT_TRUE(RqTarget.validity_state() & http::RqTarget::RQ_TARGET_BAD_PATH);
    EXPECT_EQ(RqTarget.Good(), false);
}

TEST(PathTests, 2_Test) {
    http::RqTarget RqTarget = http::RqTarget("");
    EXPECT_TRUE(RqTarget.validity_state() & http::RqTarget::RQ_TARGET_BAD_PATH);
    EXPECT_EQ(RqTarget.Good(), false);
}

TEST(PathTests, 3_Test) {
    http::RqTarget RqTarget = http::RqTarget("/ä");
    EXPECT_TRUE(RqTarget.validity_state() & http::RqTarget::RQ_TARGET_BAD_PATH);
}

TEST(PathTests, 4_Test) {
    http::RqTarget RqTarget = http::RqTarget("/bad:path");
    EXPECT_TRUE(RqTarget.validity_state() & http::RqTarget::RQ_TARGET_BAD_PATH);
}

TEST(PathTests, 5_Test) {
    http::RqTarget RqTarget = http::RqTarget("/badpath@");
    EXPECT_TRUE(RqTarget.validity_state() & http::RqTarget::RQ_TARGET_BAD_PATH);
}

TEST(PathTests, 6_Test) {
    http::RqTarget RqTarget = http::RqTarget("/badpath]/some_more");
    EXPECT_TRUE(RqTarget.validity_state() & http::RqTarget::RQ_TARGET_BAD_PATH);
}

TEST(PathTests, 7_Test) {
    http::RqTarget RqTarget = http::RqTarget("/bad path");
    EXPECT_TRUE(RqTarget.validity_state() & http::RqTarget::RQ_TARGET_BAD_PATH);
}

TEST(PathTests, 8_Test) {
    http::RqTarget RqTarget = http::RqTarget("/ok+path");
    EXPECT_EQ(RqTarget.path(), "/ok+path");
    EXPECT_EQ(RqTarget.Good(), true);
}

TEST(PathTests, 9_Test) {
    http::RqTarget RqTarget = http::RqTarget("/ok%20path");
    EXPECT_EQ(RqTarget.path(), "/ok%20path");
    EXPECT_EQ(RqTarget.Good(), true);
}

TEST(PathTests, 10_Test) {
    http::RqTarget RqTarget = http::RqTarget("/this_is_allowed/~");
    EXPECT_EQ(RqTarget.path(), "/this_is_allowed/~");
    EXPECT_EQ(RqTarget.Good(), true);
}

TEST(PathTests, 11_Test) {
    http::RqTarget RqTarget = http::RqTarget("/this+is+fine/9.9/.");
    EXPECT_EQ(RqTarget.path(), "/this+is+fine/9.9/.");
    EXPECT_EQ(RqTarget.Good(), true);
}

TEST(PathTests, 12_Test) {
    http::RqTarget RqTarget = http::RqTarget("/sub_delims_are_fine/!$&'()*+,;");
    EXPECT_EQ(RqTarget.path(), "/sub_delims_are_fine/!$&'()*+,;");
    EXPECT_EQ(RqTarget.Good(), true);
}

TEST(PathTests, 13_Test) {
    http::RqTarget RqTarget = http::RqTarget("/unreserved_chars_are_fine/abcABC123-_.~");
    EXPECT_EQ(RqTarget.path(), "/unreserved_chars_are_fine/abcABC123-_.~");
    EXPECT_EQ(RqTarget.Good(), true);
}

TEST(PathDecoding, 1_Test) {
    http::RqTarget RqTarget = http::RqTarget("/invalid_encoding%2");
    EXPECT_TRUE(RqTarget.validity_state() & http::RqTarget::RQ_TARGET_BAD_PATH);
}

TEST(PathDecoding, 2_Test) {
    http::RqTarget RqTarget = http::RqTarget("/invalid_encoding%2G");
    EXPECT_TRUE(RqTarget.validity_state() & http::RqTarget::RQ_TARGET_BAD_PATH);
}

TEST(PathDecoding, 3_Test) {
    http::RqTarget RqTarget = http::RqTarget("/invalid_encoding%Z");
    EXPECT_TRUE(RqTarget.validity_state() & http::RqTarget::RQ_TARGET_BAD_PATH);
}

TEST(PathDecoding, 4_Test) {
    http::RqTarget RqTarget = http::RqTarget("/invalid_encoding%9xabc");
    EXPECT_TRUE(RqTarget.validity_state() & http::RqTarget::RQ_TARGET_BAD_PATH);
}

TEST(PathDecoding, 5_Test) {
    http::RqTarget RqTarget = http::RqTarget("/%5A%5a%5a");
    EXPECT_EQ(RqTarget.path(), "/ZZZ");
    EXPECT_EQ(RqTarget.Good(), true);
}

TEST(PathDecoding, 6_Test) {
    http::RqTarget RqTarget = http::RqTarget("/abc%2Fdef");
    EXPECT_EQ(RqTarget.path(), "/abc%2Fdef");
    EXPECT_EQ(RqTarget.Good(), true);
}

TEST(PathDecoding, 7_Test) {
    http::RqTarget RqTarget = http::RqTarget("/abc%2fdef");
    EXPECT_EQ(RqTarget.path(), "/abc%2Fdef");
    EXPECT_EQ(RqTarget.Good(), true);
}

TEST(PathDecoding, 8_Test) {
    http::RqTarget RqTarget = http::RqTarget("/abc%2fdef%2Fghi");
    EXPECT_EQ(RqTarget.path(), "/abc%2Fdef%2Fghi");
    EXPECT_EQ(RqTarget.Good(), true);
}

TEST(PathDecoding, 9_Test) {
    http::RqTarget RqTarget = http::RqTarget("/encoded/dots/will/be/decoded%2E%2E%2E");
    EXPECT_EQ(RqTarget.path(), "/encoded/dots/will/be/decoded...");
    EXPECT_EQ(RqTarget.Good(), true);
}

///...
