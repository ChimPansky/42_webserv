#include <gtest/gtest.h>

#include "Location.h"

TEST(LocationTest, ExactMatch)
{
    std::pair<std::string, bool /* is exact match */> route("/path", true);

    std::pair<bool, std::string> result = Location::MatchUriPath("/path", route);
    EXPECT_EQ(result.first, true);
    EXPECT_EQ(result.second, "/path");

    result = Location::MatchUriPath("/pathh", route);
    EXPECT_EQ(result.first, false);
    EXPECT_EQ(result.second, "");
}

TEST(LocationTest, PrefixMatch)
{
    std::pair<std::string, bool /* is exact match */> route("/path", false);

    std::pair<bool, std::string> result = Location::MatchUriPath("/path/to/resource", route);
    EXPECT_EQ(result.first, false);
    EXPECT_EQ(result.second, "/path");

    result = Location::MatchUriPath("/path", route);
    EXPECT_EQ(result.first, false);
    EXPECT_EQ(result.second, "/path");
}

TEST(LocationTest, NoMatch)
{
    std::pair<std::string, bool /* is exact match */> route("/path", true);

    std::pair<bool, std::string> result = Location::MatchUriPath("/pat", route);
    EXPECT_EQ(result.first, false);
    EXPECT_EQ(result.second, "");

    result = Location::MatchUriPath("/path/to/resource", route);
    EXPECT_EQ(result.first, false);
    EXPECT_EQ(result.second, "");
}


TEST(LocationTest, NoPrefixMatch)
{
    std::pair<std::string, bool /* is exact match */> route("/path", false);

    std::pair<bool, std::string> result = Location::MatchUriPath("/other/path", route);
    EXPECT_EQ(result.first, false);
    EXPECT_EQ(result.second, "");

    result = Location::MatchUriPath("/pathto/resource", route);
    EXPECT_EQ(result.first, false);
    EXPECT_EQ(result.second, "");

    result = Location::MatchUriPath("/pat", route);
    EXPECT_EQ(result.first, false);
    EXPECT_EQ(result.second, "");
}
