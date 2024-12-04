#include <gtest/gtest.h>

#include "Location.h"

TEST(LocationTest, ExactMatch)
{
    std::pair<std::string, bool /* is exact match */> route("/path", true);

    std::pair<std::string, bool> result = Location::MatchUriPath("/path", route);
    EXPECT_EQ(result.first, "/path");
    EXPECT_EQ(result.second, true);

    result = Location::MatchUriPath("/pathh", route);
    EXPECT_EQ(result.first, "");
    EXPECT_EQ(result.second, false);
}

TEST(LocationTest, PrefixMatch)
{
    std::pair<std::string, bool /* is exact match */> route("/path", false);

    std::pair<std::string, bool> result = Location::MatchUriPath("/path/to/resource", route);
    EXPECT_EQ(result.first, "/path");
    EXPECT_EQ(result.second, false);

    result = Location::MatchUriPath("/path", route);
    EXPECT_EQ(result.first, "/path");
    EXPECT_EQ(result.second, false);
}

TEST(LocationTest, NoMatch)
{
    std::pair<std::string, bool /* is exact match */> route("/path", true);

    std::pair<std::string, bool> result = Location::MatchUriPath("/pat", route);
    EXPECT_EQ(result.first, "");
    EXPECT_EQ(result.second, false);

    result = Location::MatchUriPath("/path/to/resource", route);
    EXPECT_EQ(result.first, "");
    EXPECT_EQ(result.second, false);
}

TEST(LocationTest, NoPrefixMatch)
{
    std::pair<std::string, bool /* is exact match */> route("/path", false);

    std::pair<std::string, bool> result = Location::MatchUriPath("/other/path", route);
    EXPECT_EQ(result.first, "");
    EXPECT_EQ(result.second, false);

    result = Location::MatchUriPath("/pathto/resource", route);
    EXPECT_EQ(result.first, "/path");
    EXPECT_EQ(result.second, false);

    result = Location::MatchUriPath("/pat", route);
    EXPECT_EQ(result.first, "");
    EXPECT_EQ(result.second, false);
}
