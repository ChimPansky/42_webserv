#include "../src/http/Uri.h"

#include <iostream>

#include "gtest/gtest.h"

TEST(BasicTests, 1_Test)
{
    http::Uri uri = http::Uri("/path/to/file?a=20&b=30#fragment");
    EXPECT_EQ(http::Uri(uri.path(), uri.query(), uri.fragment()), uri);
    EXPECT_EQ(uri.Good(), true);
}

TEST(BasicTests, 2_Test)
{
    http::Uri uri = http::Uri("/path/to/file?a=20&b=30#fragment");
    EXPECT_EQ("/path/to/file", uri.path());
    EXPECT_EQ("a=20&b=30", uri.query());
    EXPECT_EQ("fragment", uri.fragment());
    EXPECT_EQ(uri.Good(), true);
}

TEST(BasicTests, 3_Test)
{
    http::Uri uri = http::Uri("/path/to/file?a=20&b=30#fragment");
    EXPECT_EQ("/path/to/file?a=20&b=30#fragment", uri.ToStr());
    EXPECT_EQ(uri.Good(), true);
}

TEST(BasicTests, 4_Test)
{
    http::Uri uri = http::Uri("path/to/file?a=20&b=30#fragment");
    EXPECT_EQ(uri.Good(), false);
}

TEST(BasicTests, 5_Test)
{
    http::Uri uri = http::Uri("");
    EXPECT_EQ(uri.Good(), false);
}

TEST(BasicTests, 6_Test)
{
    http::Uri uri = http::Uri(".");
    EXPECT_EQ(uri.Good(), false);
}

TEST(BasicTests, 7_Test)
{
    http::Uri uri = http::Uri("/");
    EXPECT_EQ(uri.path(), "/");
    EXPECT_EQ(uri.query(), "");
    EXPECT_EQ(uri.fragment(), "");
    EXPECT_EQ(uri.Good(), true);
}

TEST(BasicTests, 8_Test)
{
    http::Uri uri = http::Uri("?");
    EXPECT_EQ(uri.path(), "");
    EXPECT_EQ(uri.query(), "");
    EXPECT_EQ(uri.fragment(), "");
    EXPECT_EQ(uri.Good(), false);
}

TEST(BasicTests, 9_Test)
{
    http::Uri uri = http::Uri("#");
    EXPECT_EQ(uri.path(), "");
    EXPECT_EQ(uri.query(), "");
    EXPECT_EQ(uri.fragment(), "");
    EXPECT_EQ(uri.Good(), false);
}

TEST(BasicTests, 10_Test)
{
    http::Uri uri = http::Uri(" ");
    EXPECT_EQ(uri.path(), "");
    EXPECT_EQ(uri.query(), "");
    EXPECT_EQ(uri.fragment(), "");
    EXPECT_EQ(uri.Good(), false);
}

TEST(BasicTests, 11_Test)
{
    http::Uri uri = http::Uri("abc");
    EXPECT_EQ(uri.path(), "");
    EXPECT_EQ(uri.query(), "");
    EXPECT_EQ(uri.fragment(), "");
    EXPECT_EQ(uri.Good(), false);
}

TEST(PathTests, 1_Test)
{
    http::Uri uri = http::Uri("[");
    EXPECT_EQ(uri.path(), "");
    EXPECT_EQ(uri.query(), "");
    EXPECT_EQ(uri.fragment(), "");
    EXPECT_EQ(uri.Good(), false);
}

TEST(PathTests, 2_Test)
{
    http::Uri uri = http::Uri("\"");
    EXPECT_EQ(uri.path(), "");
    EXPECT_EQ(uri.query(), "");
    EXPECT_EQ(uri.fragment(), "");
    EXPECT_EQ(uri.Good(), false);
}

TEST(QueryTests, 1_Test)
{
    http::Uri uri = http::Uri("/?");
    EXPECT_EQ(uri.path(), "/");
    EXPECT_EQ(uri.query(), "");
    EXPECT_EQ(uri.fragment(), "");
    EXPECT_EQ(uri.Good(), true);
}

TEST(QueryTests, 2_Test)
{
    http::Uri uri = http::Uri("/path?a=20&b=30&c=40");
    EXPECT_EQ(uri.path(), "/path");
    EXPECT_EQ(uri.query(), "a=20&b=30&c=40");
    EXPECT_EQ(uri.fragment(), "");
    EXPECT_EQ(uri.Good(), true);
}

TEST(FragmentTests, 1_Test)
{
    http::Uri uri = http::Uri("/path#top");
    EXPECT_EQ(uri.path(), "/path");
    EXPECT_EQ(uri.query(), "");
    EXPECT_EQ(uri.fragment(), "top");
    EXPECT_EQ(uri.Good(), true);
}

TEST(FragmentTests, 2_Test)
{
    http::Uri uri = http::Uri("/path?#page=4");
    EXPECT_EQ(uri.path(), "/path");
    EXPECT_EQ(uri.query(), "");
    EXPECT_EQ(uri.fragment(), "page=4");
    EXPECT_EQ(uri.Good(), true);
}

TEST(FragmentTests, 3_Test)
{
    http::Uri uri = http::Uri("/path#\"");
    EXPECT_EQ(uri.path(), "/path");
    EXPECT_EQ(uri.query(), "");
    EXPECT_EQ(uri.fragment(), "\"");
    EXPECT_EQ(uri.Good(), true);
}

TEST(CopyTests, 1_Test_Copy_Ctor)
{
    http::Uri uri1 = http::Uri("/path/to/file");
    http::Uri uri2 = uri1;
    http::Uri uri3(uri1);
    http::Uri uri4("/path/to/file2");
    EXPECT_EQ(uri1, uri2);
    EXPECT_EQ(uri1, uri3);
    EXPECT_EQ(uri2, uri3);
    EXPECT_NE(uri1, uri4);
}

TEST(DecodeTests, 1_Test_Path)
{
    http::Uri uri = http::Uri("/path%20to%20file");
    EXPECT_EQ(uri.path(), "/path to file");
    EXPECT_EQ(uri.query(), "");
    EXPECT_EQ(uri.fragment(), "");
    EXPECT_EQ(uri.Good(), true);
}

TEST(DecodeTests, 2_Test_Path)
{
    http::Uri uri = http::Uri("/path%");
    EXPECT_EQ(uri.Good(), false);
}

TEST(DecodeTests, 3_Test_Path)
{
    http::Uri uri = http::Uri("/path%2");
    EXPECT_EQ(uri.Good(), false);
}

TEST(DecodeTests, 4_Test_Path)
{
    http::Uri uri = http::Uri("/path%2g");
    EXPECT_EQ(uri.Good(), false);
}

TEST(DecodeTests, 5_Test_Path)
{
    http::Uri uri = http::Uri("/path%%");
    EXPECT_EQ(uri.Good(), false);
}

TEST(DecodeTests, 6_Test_Path)
{
    http::Uri uri = http::Uri("/path%20%20%20");
    EXPECT_EQ(uri.path(), "/path   ");
    EXPECT_EQ(uri.query(), "");
    EXPECT_EQ(uri.fragment(), "");
    EXPECT_EQ(uri.Good(), true);
}

TEST(DecodeTests, 7_Test_Path)
{
    http::Uri uri = http::Uri("/folder%2F1/hello%20world%20?some_query=5#some_fragment");
    EXPECT_EQ(uri.path(), "/folder%2F1/hello world ");
    EXPECT_EQ(uri.query(), "some_query=5");
    EXPECT_EQ(uri.fragment(), "some_fragment");
    EXPECT_EQ(uri.Good(), true);
}

TEST(DecodeTests, 11_Test_Query)
{
    http::Uri uri = http::Uri("/path?a=20%20&b=30%20");
    EXPECT_EQ(uri.path(), "/path");
    EXPECT_EQ(uri.query(), "a=20 &b=30 ");
    EXPECT_EQ(uri.fragment(), "");
    EXPECT_EQ(uri.Good(), true);
}

TEST(DecodeTests, 12_Test_Query)
{
    http::Uri uri = http::Uri("/path?a=20&b=hello%20world&c=some%2Fpath");
    EXPECT_EQ(uri.path(), "/path");
    EXPECT_EQ(uri.query(), "a=20&b=hello world&c=some/path");
    EXPECT_EQ(uri.fragment(), "");
    EXPECT_EQ(uri.Good(), true);
}

TEST(DecodeTests, 13_Test_Query)
{
    http::Uri uri = http::Uri("/path?a=20&cartoon=Tom%20%26%20Jerry&c=some%2Fpath");
    EXPECT_EQ(uri.path(), "/path");
    EXPECT_EQ(uri.query(), "a=20&cartoon=Tom %26 Jerry&c=some/path");
    EXPECT_EQ(uri.fragment(), "");
    EXPECT_EQ(uri.Good(), true);
}

TEST(DecodeTests, 20_Test_Fragment)
{
    http::Uri uri = http::Uri("/path#top%20");
    EXPECT_EQ(uri.path(), "/path");
    EXPECT_EQ(uri.query(), "");
    EXPECT_EQ(uri.fragment(), "top ");
    EXPECT_EQ(uri.Good(), true);
}

TEST(DecodeTests, 21_Test_Fragment)
{
    http::Uri uri = http::Uri("/path#some%2Ffragment");
    EXPECT_EQ(uri.path(), "/path");
    EXPECT_EQ(uri.query(), "");
    EXPECT_EQ(uri.fragment(), "some/fragment");
    EXPECT_EQ(uri.Good(), true);
}

TEST(DecodeTests, 22_Test_Fragment)
{
    http::Uri uri = http::Uri("/path#top%01");
    EXPECT_EQ(uri.path(), "/path");
    EXPECT_EQ(uri.query(), "");
    EXPECT_EQ(uri.fragment(), "top\x01");
    EXPECT_EQ(uri.Good(), true);
}

TEST(DecodeTests, 23_Test_Fragment)
{
    http::Uri uri = http::Uri("/path#%0Aabc");
    EXPECT_EQ(uri.path(), "/path");
    EXPECT_EQ(uri.query(), "");
    EXPECT_EQ(uri.fragment(),
              "\x0A"
              "abc");
    EXPECT_EQ(uri.Good(), true);
}

TEST(NormalizeTests, 30_Test_Path)
{
    http::Uri uri = http::Uri("");
    EXPECT_EQ(uri.Good(), false);
}

TEST(NormalizeTests, 31_Test_Path)
{
    http::Uri uri = http::Uri("/");
    EXPECT_EQ(uri.path(), "/");
    EXPECT_EQ(uri.Good(), true);
}

TEST(NormalizeTests, 32_Test_Path)
{
    http::Uri uri = http::Uri("/.");
    EXPECT_EQ(uri.path(), "/");
    EXPECT_EQ(uri.Good(), true);
}

TEST(NormalizeTests, 33_Test_Path)
{
    http::Uri uri = http::Uri("/abc/.");
    EXPECT_EQ(uri.path(), "/abc/");
    EXPECT_EQ(uri.Good(), true);
}

TEST(NormalizeTests, 34_Test_Path)
{
    http::Uri uri = http::Uri("/abc/./def");
    EXPECT_EQ(uri.path(), "/abc/def");
    EXPECT_EQ(uri.Good(), true);
}

TEST(NormalizeTests, 35_Test_Path)
{
    http::Uri uri = http::Uri("/abc/./def/.");
    EXPECT_EQ(uri.path(), "/abc/def/");
    EXPECT_EQ(uri.Good(), true);
}

TEST(NormalizeTests, 36_Test_Path)
{
    http::Uri uri = http::Uri("/./abc");
    EXPECT_EQ(uri.path(), "/abc");
    EXPECT_EQ(uri.Good(), true);
}

TEST(NormalizeTests, 37_Test_Path)
{
    http::Uri uri = http::Uri("/.");
    EXPECT_EQ(uri.path(), "/");
    EXPECT_EQ(uri.Good(), true);
}

TEST(NormalizeTests, 38_Test_Path)
{
    http::Uri uri = http::Uri("/./");
    EXPECT_EQ(uri.path(), "/");
    EXPECT_EQ(uri.Good(), true);
}

TEST(NormalizeTests, 39_Test_Path)
{
    http::Uri uri = http::Uri("..");
    EXPECT_EQ(uri.Good(), false);
}

TEST(NormalizeTests, 40_Test_Path)
{
    http::Uri uri = http::Uri(".");
    EXPECT_EQ(uri.Good(), false);
}

TEST(NormalizeTests, 41_Test_Path)
{
    http::Uri uri = http::Uri("/abc/def/../ghi/../jkl");
    EXPECT_EQ(uri.path(), "/abc/jkl");
    EXPECT_EQ(uri.Good(), true);
}

TEST(NormalizeTests, 42_Test_Path)
{
    http::Uri uri = http::Uri("/abc%2Fdef/ghi/../jkl/");
    EXPECT_EQ(uri.path(), "/abc%2Fdef/jkl/");
    EXPECT_EQ(uri.Good(), true);
}

TEST(NormalizeTests, 43_Test_Path)
{
    http::Uri uri = http::Uri("/abc%2Fdef/ghi/../jkl");
    EXPECT_EQ(uri.path(), "/abc%2Fdef/jkl");
    EXPECT_EQ(uri.Good(), true);
}

TEST(NormalizeTests, 44_Test_Path)
{
    http::Uri uri = http::Uri("/abc%2Fdef/ghi/../jkl%2F");
    EXPECT_EQ(uri.path(), "/abc%2Fdef/jkl%2F");
    EXPECT_EQ(uri.Good(), true);
}

TEST(NormalizeTests, 45_Test_Path)
{
    http::Uri uri = http::Uri("/abc%2Fdef/ghi/../jkl%2Fmno");
    EXPECT_EQ(uri.path(), "/abc%2Fdef/jkl%2Fmno");
    EXPECT_EQ(uri.Good(), true);
}

TEST(NormalizeTests, 46_Test_Path)
{
    http::Uri uri = http::Uri("/abc%2Fdef/ghi/../jkl%2Fmno/../../../../");
    EXPECT_EQ(uri.Good(), false);
}

TEST(NormalizeTests, 47_Test_Path)
{
    http::Uri uri = http::Uri("/abc/def/../ghi/../../jkl");
    EXPECT_EQ(uri.path(), "/jkl");
    EXPECT_EQ(uri.Good(), true);
}

TEST(NormalizeTests, 48_Test_Path)
{
    http::Uri uri = http::Uri("/a/b/..c/");
    EXPECT_EQ(uri.path(), "/a/b/..c/");
    EXPECT_EQ(uri.Good(), true);
}

TEST(NormalizeTests, 49_Test_Path)
{
    http::Uri uri = http::Uri("/a/b/.c/");
    EXPECT_EQ(uri.path(), "/a/b/.c/");
    EXPECT_EQ(uri.Good(), true);
}

TEST(NormalizeTests, 50_Test_Path)
{
    http::Uri uri = http::Uri("/a/./b/.c/");
    EXPECT_EQ(uri.path(), "/a/b/.c/");
    EXPECT_EQ(uri.Good(), true);
}

TEST(CollapseTests, 60_Test_Collapse)
{
    http::Uri uri = http::Uri("/////");
    EXPECT_EQ(uri.path(), "/");
    EXPECT_EQ(uri.Good(), true);
}

TEST(CollapseTests, 61_Test_Collapse)
{
    http::Uri uri = http::Uri("/");
    EXPECT_EQ(uri.path(), "/");
    EXPECT_EQ(uri.Good(), true);
}

TEST(CollapseTests, 62_Test_Collapse)
{
    http::Uri uri = http::Uri("/abc/def//");
    EXPECT_EQ(uri.path(), "/abc/def/");
    EXPECT_EQ(uri.Good(), true);
}

TEST(CollapseTests, 63_Test_Collapse)
{
    http::Uri uri = http::Uri("/abc/def//ghi");
    EXPECT_EQ(uri.path(), "/abc/def/ghi");
    EXPECT_EQ(uri.Good(), true);
}

TEST(CollapseTests, 64_Test_Collapse)
{
    http::Uri uri = http::Uri("///////abc/def//ghi//");
    EXPECT_EQ(uri.path(), "/abc/def/ghi/");
    EXPECT_EQ(uri.Good(), true);
}

TEST(CollapseTests, 65_Test_Collapse)
{
    http::Uri uri = http::Uri("/%2F//////abc/def//ghi//jkl");
    EXPECT_EQ(uri.path(), "/%2F/abc/def/ghi/jkl");
    EXPECT_EQ(uri.Good(), true);
}

TEST(ComparisonTests, 80_Test_Comparison)
{
    http::Uri uri1 = http::Uri("/a/b/c/%7Bfoo%7D");
    http::Uri uri2 = http::Uri("/a/./b/../b/%63/%7bfoo%7d");
    EXPECT_EQ(uri1, uri2);
}

TEST(ComparisonTests, 81_Test_Comparison)
{
    http::Uri uri1 = http::Uri("/a/b/c/%7Bfoo%7D");
    http::Uri uri2 = http::Uri("/a/./b/../b/%63/%7bfoo%7d");
    EXPECT_EQ(uri1, uri2);
}

TEST(ComparisonTests, 82_Test_Comparison)
{
    http::Uri uri1 = http::Uri("/a/b/%2E%2E/c/d");
    http::Uri uri2 = http::Uri("/a/c/d");
    EXPECT_EQ(uri1, uri2);
}

TEST(ComparisonTests, 83_Test_Comparison)
{
    http::Uri uri1 = http::Uri("/a/%2e/b");
    http::Uri uri2 = http::Uri("/a/b");
    EXPECT_EQ(uri1, uri2);
}

TEST(ComparisonTests, 84_Test_Comparison)
{
    http::Uri uri1 = http::Uri("/a/b/.././c");
    http::Uri uri2 = http::Uri("/a/c");
    EXPECT_EQ(uri1, uri2);
}

TEST(ComparisonTests, 86_Test_Comparison)
{
    http::Uri uri1 = http::Uri("/a/b/c/%41");
    http::Uri uri2 = http::Uri("/a/b/c/A");
    EXPECT_EQ(uri1, uri2);
}

// TEST(ComparisonTests, 87_Test_Comparison) {
//     http::Uri uri1 = http::Uri("/a/b/%7e");
//     http::Uri uri2 = http::Uri("/a/b/~");
//     EXPECT_EQ(uri1, uri2);
// }

TEST(ComparisonTests, 88_Test_Comparison)
{
    http::Uri uri1 = http::Uri("/a/b/c//d/");
    http::Uri uri2 = http::Uri("/a/b/c/d/");
    EXPECT_EQ(uri1, uri2);
}
