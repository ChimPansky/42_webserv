#include <cstring>
#include "gtest/gtest.h"
#include "../src/http/RequestBuilder.h"
#include <fstream>

#define BODY_14 "Hello, World!!"

#define BODY_1100 "Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam erat, sed diam voluptua. At vero eos et accusam et justo duo dolores et ea rebum. Stet clita kasd gubergren, no sea takimata sanctus est Lorem ipsum dolor sit amet. Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam erat, sed diam voluptua. At vero eos et accusam et justo duo dolores et ea rebum. Stet clita kasd gubergren, no sea takimata sanctus est Lorem ipsum dolor sit amet. Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam erat, sed diam voluptua. At vero eos et accusam et justo duo dolores et ea rebum. Stet clita kasd gubergren, no sea takimata sanctus est Lorem ipsum dolor sit amet. Duis autem vel eum iriure dolor in hendrerit in vulputate velit esse molestie consequat, vel illum dolore eu feugiat nulla facilisis at vero eros et accumsan et iusto odio dignissim qui blandit praesent luptat!!!"

static size_t ReadFromFile(std::ifstream& file, std::vector<char>& buf, size_t sz) {
    size_t old_sz = buf.size();
    buf.resize(old_sz + sz);
    file.read(buf.data() + old_sz, sz);
    return file.gcount();
}

static int BuildRequest(http::RequestBuilder& builder, const char* rq_path, size_t read_size = 10) {
    std::ifstream file(rq_path);
    if (!file.is_open()) {
        std::cerr << "Could not open Request File: " << rq_path << std::endl;
        return 1;
    }
    size_t bytes_read = 0;
    while (!builder.IsReadyForResponse()) {
        bytes_read = 0;
        if (builder.needs_info_from_server()) {
            //std::cout << "RequestBuilder needs info from server -> Get it and then continue building..." << std::endl;
            // these settings should come from server:
            builder.set_max_body_size(10000);  // 1MB
        }
        if (builder.HasReachedEndOfBuffer()) {
            bytes_read = ReadFromFile(file, builder.buf(), read_size);
            builder.buf().resize(builder.buf().size() - (read_size - bytes_read));
        }
        if (builder.ProcessBuffer(bytes_read) == 0) {
            // std::cerr << "ProcessBuffer returned 0..." << std::endl;
            break;
        }
    }
    return 0;
}

TEST(ValidWithBody, 1_Bodylen_14_Buffer_50) {
    http::RequestBuilder builder;
    if (BuildRequest(builder, "requests/rq1.txt", 50) != 0) {
        FAIL();
    }
    ASSERT_EQ(http::HTTP_POST, builder.rq().method);
    ASSERT_EQ("/", builder.rq().uri);
    ASSERT_EQ(http::HTTP_1_1, builder.rq().version);
    ASSERT_EQ("www.example.com", builder.rq().GetHeaderVal("host"));
    ASSERT_EQ("14", builder.rq().GetHeaderVal("content-length"));
    ASSERT_EQ((unsigned long)14, builder.rq().body.size());
    const char* str = BODY_14;
    ASSERT_EQ(std::vector<char>(str, str + strlen(str)), builder.rq().body);
    ASSERT_EQ(http::RQ_GOOD, builder.rq().status);
}

TEST(ValidWithBody, 2_One_Chunk_1100_Buffer_10) {
    http::RequestBuilder builder;
    if (BuildRequest(builder, "requests/rq2.txt", 10) != 0) {
        FAIL();
    }
    ASSERT_EQ(http::HTTP_POST, builder.rq().method);
    ASSERT_EQ("/upload", builder.rq().uri);
    ASSERT_EQ(http::HTTP_1_1, builder.rq().version);
    ASSERT_EQ("chunked", builder.rq().GetHeaderVal("transfer-encoding"));
    ASSERT_EQ((unsigned long)1100, builder.rq().body.size());
    const char* str = BODY_1100;
    ASSERT_EQ(std::vector<char>(str, str + strlen(str)), builder.rq().body);
    ASSERT_EQ(http::RQ_GOOD, builder.rq().status);
}

TEST(ValidWithBody, 2_One_Chunk_1100_Buffer_9) {
    http::RequestBuilder builder;
    if (BuildRequest(builder, "requests/rq2.txt", 9) != 0) {
        FAIL();
    }
    ASSERT_EQ(http::HTTP_POST, builder.rq().method);
    ASSERT_EQ("/upload", builder.rq().uri);
    ASSERT_EQ(http::HTTP_1_1, builder.rq().version);
    ASSERT_EQ("chunked", builder.rq().GetHeaderVal("transfer-encoding"));
    ASSERT_EQ((unsigned long)1100, builder.rq().body.size());
    const char* str = BODY_1100;
    ASSERT_EQ(std::vector<char>(str, str + strlen(str)), builder.rq().body);
    ASSERT_EQ(http::RQ_GOOD, builder.rq().status);
}

TEST(ValidWithBody, 4_Bodylen_1_Buffer_50) {
    http::RequestBuilder builder;
    if (BuildRequest(builder, "requests/rq4.txt", 50) != 0) {
        FAIL();
    }
    ASSERT_EQ(http::HTTP_POST, builder.rq().method);
    ASSERT_EQ("/", builder.rq().uri);
    ASSERT_EQ(http::HTTP_1_1, builder.rq().version);
    ASSERT_EQ("www.example.com", builder.rq().GetHeaderVal("host"));
    ASSERT_EQ("1", builder.rq().GetHeaderVal("content-length"));
    ASSERT_EQ((unsigned long)1, builder.rq().body.size());
    const char *str = "a";
    ASSERT_EQ(std::vector<char>(str, str + strlen(str)), builder.rq().body);
    ASSERT_EQ(http::RQ_GOOD, builder.rq().status);
}

TEST(ValidWithBody, 5_Chunked_1_Buffer_10) {
    http::RequestBuilder builder;
    if (BuildRequest(builder, "requests/rq5.txt", 50) != 0) {
        FAIL();
    }
    ASSERT_EQ(http::HTTP_POST, builder.rq().method);
    ASSERT_EQ("/upload", builder.rq().uri);
    ASSERT_EQ(http::HTTP_1_0, builder.rq().version);
    ASSERT_EQ("chunked", builder.rq().GetHeaderVal("transfer-encoding"));
    const char *str = "L";
    ASSERT_EQ(std::vector<char>(str, str + strlen(str)), builder.rq().body);
    ASSERT_EQ(http::RQ_GOOD, builder.rq().status);
}

// Valid without body:
TEST(ValidWithoutBody, 6_SimpleGet_Buffer_50) {
    http::RequestBuilder builder;
    if (BuildRequest(builder, "requests/rq6.txt", 50) != 0) {
        FAIL();
    }
    ASSERT_EQ(http::HTTP_GET, builder.rq().method);
    ASSERT_EQ("/", builder.rq().uri);
    ASSERT_EQ(http::HTTP_1_1, builder.rq().version);
    ASSERT_EQ("www.example.com", builder.rq().GetHeaderVal("host"));
    ASSERT_EQ("", builder.rq().GetHeaderVal("content-length"));
    ASSERT_TRUE(builder.rq().body.empty());
    ASSERT_EQ(http::RQ_GOOD, builder.rq().status);
}


TEST(ValidWithoutBody, 7_GetWithQuery_Buffer_100) {
    http::RequestBuilder builder;
    if (BuildRequest(builder, "requests/rq7.txt", 100) != 0) {
        FAIL();
    }
    ASSERT_EQ(http::HTTP_GET, builder.rq().method);
    ASSERT_EQ("/search?q=example", builder.rq().uri);
    ASSERT_EQ(http::HTTP_1_1, builder.rq().version);
    ASSERT_EQ("www.search.com", builder.rq().GetHeaderVal("host"));
    ASSERT_EQ("", builder.rq().GetHeaderVal("content-length"));
    ASSERT_TRUE(builder.rq().body.empty());
    ASSERT_EQ(http::RQ_GOOD, builder.rq().status);
}

TEST(ValidWithoutBody, 8_GetWithHeaders_Buffer_100) {
    http::RequestBuilder builder;
    if (BuildRequest(builder, "requests/rq8.txt", 100) != 0) {
        FAIL();
    }
    ASSERT_EQ(http::HTTP_GET, builder.rq().method);
    ASSERT_EQ("/products", builder.rq().uri);
    ASSERT_EQ(http::HTTP_1_1, builder.rq().version);
    ASSERT_EQ("shop.example.com", builder.rq().GetHeaderVal("host"));
    ASSERT_EQ("application/json", builder.rq().GetHeaderVal("accept"));
    ASSERT_EQ("CustomClient/1.0", builder.rq().GetHeaderVal("user-agent"));
    ASSERT_TRUE(builder.rq().body.empty());
    ASSERT_EQ(http::RQ_GOOD, builder.rq().status);
}

TEST(InValidWithoutBody, 9_PostWithHeaders_Buffer_80) {
    http::RequestBuilder builder;
    if (BuildRequest(builder, "requests/rq9.txt", 80) != 0) {
        FAIL();
    }
    ASSERT_EQ(http::HTTP_POST, builder.rq().method);
    ASSERT_EQ("/submit", builder.rq().uri);
    ASSERT_EQ(http::HTTP_1_1, builder.rq().version);
    ASSERT_EQ("www.example.com", builder.rq().GetHeaderVal("host"));
    ASSERT_EQ("application/x-www-form-urlencoded", builder.rq().GetHeaderVal("content-type"));
    ASSERT_EQ("http://www.example.com", builder.rq().GetHeaderVal("referer"));
    ASSERT_TRUE(builder.rq().body.empty());
    ASSERT_EQ(http::RQ_BAD, builder.rq().status);
}

TEST(ValidWithoutBody, 10_DeleteWithHeaders_Buffer_50) {
    http::RequestBuilder builder;
    if (BuildRequest(builder, "requests/rq10.txt", 50) != 0) {
        FAIL();
    }
    ASSERT_EQ(http::HTTP_DELETE, builder.rq().method);
    ASSERT_EQ("/items/123", builder.rq().uri);
    ASSERT_EQ(http::HTTP_1_0, builder.rq().version);
    ASSERT_EQ("api.items.com", builder.rq().GetHeaderVal("host"));
    ASSERT_EQ("Bearer some_token", builder.rq().GetHeaderVal("authorization"));
    ASSERT_TRUE(builder.rq().body.empty());
    ASSERT_EQ(http::RQ_GOOD, builder.rq().status);
}

TEST(InValidWithoutBody, 11_Incomplete_Method_Buffer_7) {
    http::RequestBuilder builder;
    if (BuildRequest(builder, "requests/rq11.txt", 7) != 0) {
        FAIL();
    }
    ASSERT_EQ(http::RQ_BAD, builder.rq().status);
}

TEST(InValidWithoutBody, 12_Too_Many_Spaces_Buffer_7) {
    http::RequestBuilder builder;
    if (BuildRequest(builder, "requests/rq12.txt", 7) != 0) {
        FAIL();
    }
    ASSERT_EQ(http::HTTP_GET, builder.rq().method);
    ASSERT_EQ(http::RQ_BAD, builder.rq().status);
}

TEST(InValidWithoutBody, 13_No_URI_Buffer_10) {
    http::RequestBuilder builder;
    if (BuildRequest(builder, "requests/rq13.txt", 10) != 0) {
        FAIL();
    }
    ASSERT_EQ(http::HTTP_GET, builder.rq().method);
    ASSERT_TRUE(builder.rq().uri.empty());
    ASSERT_EQ(http::HTTP_NO_VERSION, builder.rq().version);
    ASSERT_EQ(http::RQ_BAD, builder.rq().status);
}

TEST(InValidWithoutBody, 14_No_Invalid_Version_Buffer_1000) {
    http::RequestBuilder builder;
    if (BuildRequest(builder, "requests/rq14.txt", 1000) != 0) {
        FAIL();
    }
    ASSERT_EQ(http::HTTP_GET, builder.rq().method);
    ASSERT_EQ("/upload", builder.rq().uri);
    ASSERT_EQ(http::HTTP_NO_VERSION, builder.rq().version);
    ASSERT_EQ(http::RQ_BAD, builder.rq().status);
}

// when testing this and viewing rq15.txt in editor: careful about VS Code setting "Files: Insert Final Newline"
TEST(InValidWithoutBody, 15_No_CRLF_After_Version_Buffer_10000) {
    http::RequestBuilder builder;
    if (BuildRequest(builder, "requests/rq15.txt", 10000) != 0) {
        FAIL();
    }
    ASSERT_EQ(http::HTTP_DELETE, builder.rq().method);
    ASSERT_EQ("/upload/subfolder1/subfolder2", builder.rq().uri);
    ASSERT_EQ(http::HTTP_NO_VERSION, builder.rq().version);
    ASSERT_EQ(http::RQ_BAD, builder.rq().status);
}

TEST(InValidWithoutBody, 16_Just_LF_After_Version_Buffer_13) {
    http::RequestBuilder builder;
    if (BuildRequest(builder, "requests/rq16.txt", 13) != 0) {
        FAIL();
    }
    ASSERT_EQ(http::HTTP_DELETE, builder.rq().method);
    ASSERT_EQ("/upload/subfolder1/subfolder2", builder.rq().uri);
    ASSERT_EQ(http::HTTP_NO_VERSION, builder.rq().version);
    ASSERT_EQ(http::RQ_BAD, builder.rq().status);
}

TEST(InValidWithoutBody, 17_Bad_Header_Key_Buffer_9) {
    http::RequestBuilder builder;
    if (BuildRequest(builder, "requests/rq17.txt", 9) != 0) {
        FAIL();
    }
    ASSERT_EQ(http::HTTP_DELETE, builder.rq().method);
    ASSERT_EQ("/upload/subfolder1/subfolder2", builder.rq().uri);
    ASSERT_EQ(http::HTTP_2, builder.rq().version);
    ASSERT_EQ(http::RQ_BAD, builder.rq().status);
}

TEST(InValidWithoutBody, 18_Bad_Header_Key_Buffer_1) {
    http::RequestBuilder builder;
    if (BuildRequest(builder, "requests/rq18.txt", 1) != 0) {
        FAIL();
    }
    ASSERT_EQ(http::HTTP_DELETE, builder.rq().method);
    ASSERT_EQ("/upload/subfolder1/subfolder2", builder.rq().uri);
    ASSERT_EQ(http::HTTP_2, builder.rq().version);
    ASSERT_EQ(http::RQ_BAD, builder.rq().status);
}

TEST(InValidWithoutBody, 19_Bad_Header_Key_Buffer_100) {
    http::RequestBuilder builder;
    if (BuildRequest(builder, "requests/rq19.txt", 100) != 0) {
        FAIL();
    }
    ASSERT_EQ(http::HTTP_GET, builder.rq().method);
    ASSERT_EQ("/", builder.rq().uri);
    ASSERT_EQ(http::HTTP_1_1, builder.rq().version);
    ASSERT_TRUE(builder.rq().GetHeaderVal("Host!").empty());
    ASSERT_EQ(http::RQ_BAD, builder.rq().status);
}

TEST(InValidWithoutBody, 20_Bad_Header_Key_Buffer_100) {
    http::RequestBuilder builder;
    if (BuildRequest(builder, "requests/rq20.txt", 100) != 0) {
        FAIL();
    }
    ASSERT_EQ(http::HTTP_GET, builder.rq().method);
    ASSERT_EQ("/", builder.rq().uri);
    ASSERT_EQ(http::HTTP_1_1, builder.rq().version);
    ASSERT_TRUE(builder.rq().GetHeaderVal("Host-").empty());
    ASSERT_EQ(http::RQ_BAD, builder.rq().status);
}

TEST(InValidWithoutBody, 21_Bad_Header_Key_Buffer_100) {
    http::RequestBuilder builder;
    if (BuildRequest(builder, "requests/rq21.txt", 100) != 0) {
        FAIL();
    }
    ASSERT_EQ(http::HTTP_GET, builder.rq().method);
    ASSERT_EQ("/", builder.rq().uri);
    ASSERT_EQ(http::HTTP_1_1, builder.rq().version);
    ASSERT_TRUE(builder.rq().GetHeaderVal("Host ").empty());
    ASSERT_EQ(http::RQ_BAD, builder.rq().status);
}

TEST(InValidWithoutBody, 22_Missing_Space_Buffer_70) {
    http::RequestBuilder builder;
    if (BuildRequest(builder, "requests/rq22.txt", 70) != 0) {
        FAIL();
    }
    ASSERT_EQ(http::HTTP_GET, builder.rq().method);
    ASSERT_EQ("/", builder.rq().uri);
    ASSERT_EQ(http::HTTP_1_1, builder.rq().version);
    ASSERT_TRUE(builder.rq().GetHeaderVal("Host").empty());
    ASSERT_EQ(http::RQ_BAD, builder.rq().status);
}

TEST(InValidWithoutBody, 23_Missing_Header_Value_Buffer_70) {
    http::RequestBuilder builder;
    if (BuildRequest(builder, "requests/rq23.txt", 70) != 0) {
        FAIL();
    }
    ASSERT_EQ(http::HTTP_GET, builder.rq().method);
    ASSERT_EQ("/", builder.rq().uri);
    ASSERT_EQ(http::HTTP_1_1, builder.rq().version);
    ASSERT_TRUE(builder.rq().GetHeaderVal("Host").empty());
    ASSERT_EQ(http::RQ_BAD, builder.rq().status);
}

TEST(InValidWithoutBody, 24_No_CRLF_After_Header_Value_Buffer_10) {
    http::RequestBuilder builder;
    if (BuildRequest(builder, "requests/rq24.txt", 10) != 0) {
        FAIL();
    }
    ASSERT_EQ(http::HTTP_GET, builder.rq().method);
    ASSERT_EQ("/", builder.rq().uri);
    ASSERT_EQ(http::HTTP_1_1, builder.rq().version);
    ASSERT_TRUE(builder.rq().GetHeaderVal("Host").empty());
    ASSERT_EQ(http::RQ_BAD, builder.rq().status);
}

//TEST(InValidWithoutBody, 25_Bad_Header_Value_Buffer_10) {
// TODO: Tests 25-30 check headervalues with invalid characters


//TODO: Tests 31+ check for conflicting headers? multiple headers with same key?


//TODO: Tests 41+ check for invalid body content

int main (int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
