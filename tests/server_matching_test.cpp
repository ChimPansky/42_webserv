#include <Server.h>
#include <gtest/gtest.h>

TEST(ServerTest, ExactMatch)
{
    std::vector<std::string> server_names = {"server1", "server2", "server3"};

    std::pair<MatchType, std::string> result = Server::MatchHostName("server1", server_names);
    EXPECT_EQ(result.first, EXACT_MATCH);
    EXPECT_EQ(result.second, "server1");

    result = Server::MatchHostName("server2", server_names);
    EXPECT_EQ(result.first, EXACT_MATCH);
    EXPECT_EQ(result.second, "server2");

    result = Server::MatchHostName("server3", server_names);
    EXPECT_EQ(result.first, EXACT_MATCH);
    EXPECT_EQ(result.second, "server3");
}

TEST(ServerTest, MultipleMatchingPatterns)
{
    std::vector<std::string> server_names = {"*.example.com", "www.example.com", "example.*"};

    std::pair<MatchType, std::string> result =
        Server::MatchHostName("www.example.com", server_names);
    EXPECT_EQ(result.first, EXACT_MATCH);
    EXPECT_EQ(result.second, "www.example.com");
}

TEST(ServerTest, WildcardWithEmptyHost)
{
    std::vector<std::string> server_names = {"*"};

    std::pair<MatchType, std::string> result = Server::MatchHostName("", server_names);
    EXPECT_EQ(result.first, NO_MATCH);
    EXPECT_EQ(result.second, "");
}

TEST(ServerTest, PrefixMatch)
{
    std::vector<std::string> server_names = {"*.example.com"};

    std::pair<MatchType, std::string> result =
        Server::MatchHostName("www.example.com", server_names);
    EXPECT_EQ(result.first, PREFIX_MATCH);
    EXPECT_EQ(result.second, "*.example.com");

    result = Server::MatchHostName("www.sub.example.com", server_names);
    EXPECT_EQ(result.first, PREFIX_MATCH);
    EXPECT_EQ(result.second, "*.example.com");
}

TEST(ServerTest, SuffixMatch)
{
    std::vector<std::string> server_names = {"example.*"};

    std::pair<MatchType, std::string> result =
        Server::MatchHostName("example.com.ua", server_names);
    EXPECT_EQ(result.first, SUFFIX_MATCH);
    EXPECT_EQ(result.second, "example.*");

    result = Server::MatchHostName("example.org", server_names);
    EXPECT_EQ(result.first, SUFFIX_MATCH);
    EXPECT_EQ(result.second, "example.*");
}

TEST(ServerTest, NoMatch)
{
    std::vector<std::string> server_names = {"server1", "*.example.com", "example.*"};

    std::pair<MatchType, std::string> result =
        Server::MatchHostName("nonexistent.com", server_names);
    EXPECT_EQ(result.first, NO_MATCH);
    EXPECT_EQ(result.second, "");

    result = Server::MatchHostName("sub.example.co", server_names);
    EXPECT_EQ(result.first, NO_MATCH);
    EXPECT_EQ(result.second, "");
}

TEST(ServerTest, SpecialCases)
{
    std::vector<std::string> server_names = {"www.example.com"};

    std::pair<MatchType, std::string> result =
        Server::MatchHostName("www.examplecom", server_names);
    EXPECT_EQ(result.first, NO_MATCH);
    EXPECT_EQ(result.second, "");

    server_names = {"*example.com"};
    result = Server::MatchHostName("sub.sub.example.com", server_names);
    EXPECT_EQ(result.first, PREFIX_MATCH);
    EXPECT_EQ(result.second, "*example.com");

    server_names = {"example.com*"};
    result = Server::MatchHostName("example.com", server_names);
    EXPECT_EQ(result.first, NO_MATCH);
    EXPECT_EQ(result.second, "");

    result = Server::MatchHostName("example.com.br", server_names);
    EXPECT_EQ(result.first, SUFFIX_MATCH);
    EXPECT_EQ(result.second, "example.com*");
}

TEST(ServerTest, EmptyHostOrServerName)
{
    std::vector<std::string> server_names = {"server1", "server2"};

    std::pair<MatchType, std::string> result = Server::MatchHostName("", server_names);
    EXPECT_EQ(result.first, NO_MATCH);
    EXPECT_EQ(result.second, "");

    server_names = {""};
    result = Server::MatchHostName("server1", server_names);
    EXPECT_EQ(result.first, NO_MATCH);
    EXPECT_EQ(result.second, "");
}
