#include <cstring>
#include "gtest/gtest.h"
#include "../src/http/RequestBuilder.h"
#include <fstream>

#define BODY_14 "Hello, World!!"

#define BODY_1100 "Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam erat, sed diam voluptua. At vero eos et accusam et justo duo dolores et ea rebum. Stet clita kasd gubergren, no sea takimata sanctus est Lorem ipsum dolor sit amet. Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam erat, sed diam voluptua. At vero eos et accusam et justo duo dolores et ea rebum. Stet clita kasd gubergren, no sea takimata sanctus est Lorem ipsum dolor sit amet. Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam erat, sed diam voluptua. At vero eos et accusam et justo duo dolores et ea rebum. Stet clita kasd gubergren, no sea takimata sanctus est Lorem ipsum dolor sit amet. Duis autem vel eum iriure dolor in hendrerit in vulputate velit esse molestie consequat, vel illum dolore eu feugiat nulla facilisis at vero eros et accumsan et iusto odio dignissim qui blandit praesent luptat!!!"

http::RequestBuilder* BUILDER = NULL;

size_t Recv(std::ifstream& file, std::vector<char>& buf, size_t read_sz) {
    size_t old_buf_sz = buf.size();
    buf.resize(old_buf_sz + read_sz);
    file.read(buf.data() + old_buf_sz, read_sz);
    size_t bytes_recvd = file.gcount();
    if (bytes_recvd >= 0 && static_cast<size_t>(bytes_recvd) < read_sz) {
        buf.resize(old_buf_sz + bytes_recvd);
    }
    return bytes_recvd;
}

void ProcessNewData(size_t bytes_recvd) {
    BUILDER->Build(bytes_recvd);
    size_t i = 0;
    while (BUILDER->builder_status() == http::RB_BUILDING && i < 20) {
        BUILDER->Build(bytes_recvd);
        if (BUILDER->builder_status() == http::RB_NEED_INFO_FROM_SERVER) {
            BUILDER->ApplyServerInfo(1000);
        }
        i++;
    }
}

void Call(std::ifstream& file, size_t read_sz) {
    size_t bytes_recvd = Recv(file, BUILDER->buf(), read_sz);
    ProcessNewData(bytes_recvd);
}

int BuildRequest(const char* rq_path, size_t read_size = 10) {
    std::cout << "BuildRequest" << std::endl;
    delete BUILDER;
    BUILDER = new http::RequestBuilder();
    std::ifstream file(rq_path);
    if (!file.is_open()) {
        std::cerr << "Could not open Request File: " << rq_path << std::endl;
        return 1;
    }
    while (BUILDER->builder_status() != http::RB_DONE)
        Call(file, read_size);
    BUILDER->rq().Print();
    return 0;
}



// TEST(ValidWithBody, 1_Bodylen_14_Buffer_50) {
//     if (BuildRequest("requests/rq1.txt", 50) != 0) {
//         FAIL();
//     }
//     ASSERT_EQ(http::HTTP_POST, BUILDER.rq().method);
//     ASSERT_EQ("/", BUILDER.rq().uri);
//     ASSERT_EQ(http::HTTP_1_1, BUILDER.rq().version);
//     ASSERT_EQ("www.example.com", BUILDER.rq().GetHeaderVal("host"));
//     ASSERT_EQ("14", BUILDER.rq().GetHeaderVal("content-length"));
//     const char* str = BODY_14;
//     ASSERT_EQ(std::vector<char>(str, str + strlen(str)), BUILDER.rq().body);
//     ASSERT_EQ((unsigned long)14, BUILDER.rq().body.size());
//     ASSERT_EQ(http::RQ_GOOD, BUILDER.rq().status);
// }

TEST(ValidWithBody, 2_One_Chunk_1100_Buffer_10) {
    std::cout << "Test2" << std::endl;
    if (BuildRequest("requests/rq2.txt", 10) != 0) {
        FAIL();
    }
    ASSERT_EQ(http::HTTP_POST, BUILDER->rq().method);
    ASSERT_EQ("/upload", BUILDER->rq().uri);
    ASSERT_EQ(http::HTTP_1_1, BUILDER->rq().version);
    ASSERT_EQ("chunked", BUILDER->rq().GetHeaderVal("transfer-encoding"));
    ASSERT_EQ((unsigned long)1100, BUILDER->rq().body.size());
    const char* str = BODY_1100;
    ASSERT_EQ(std::vector<char>(str, str + strlen(str)), BUILDER->rq().body);
    ASSERT_EQ(http::RQ_GOOD, BUILDER->rq().status);
}

TEST(ValidWithBody, 3_One_Chunk_1100_Buffer_9) {
    std::cout << "Test3" << std::endl;
    if (BuildRequest("requests/rq2.txt", 9) != 0) {
        FAIL();
    }
    ASSERT_EQ(http::HTTP_POST, BUILDER->rq().method);
    ASSERT_EQ("/upload", BUILDER->rq().uri);
    ASSERT_EQ(http::HTTP_1_1, BUILDER->rq().version);
    ASSERT_EQ("chunked", BUILDER->rq().GetHeaderVal("transfer-encoding"));
    ASSERT_EQ((unsigned long)1100, BUILDER->rq().body.size());
    const char* str = BODY_1100;
    ASSERT_EQ(std::vector<char>(str, str + strlen(str)), BUILDER->rq().body);
    ASSERT_EQ(http::RQ_GOOD, BUILDER->rq().status);
}

// TEST(ValidWithBody, 4_Bodylen_1_Buffer_50) {
//     if (BuildRequest("requests/rq4.txt", 50) != 0) {
//         FAIL();
//     }
//     ASSERT_EQ(http::HTTP_POST, BUILDER.rq().method);
//     ASSERT_EQ("/", BUILDER.rq().uri);
//     ASSERT_EQ(http::HTTP_1_1, BUILDER.rq().version);
//     ASSERT_EQ("www.example.com", BUILDER.rq().GetHeaderVal("host"));
//     ASSERT_EQ("1", BUILDER.rq().GetHeaderVal("content-length"));
//     ASSERT_EQ((unsigned long)1, BUILDER.rq().body.size());
//     const char *str = "a";
//     ASSERT_EQ(std::vector<char>(str, str + strlen(str)), BUILDER.rq().body);
//     ASSERT_EQ(http::RQ_GOOD, BUILDER.rq().status);
// }

// TEST(ValidWithBody, 5_Chunked_1_Buffer_10) {
//     if (BuildRequest("requests/rq5.txt", 50) != 0) {
//         FAIL();
//     }
//     ASSERT_EQ(http::HTTP_POST, BUILDER.rq().method);
//     ASSERT_EQ("/upload", BUILDER.rq().uri);
//     ASSERT_EQ(http::HTTP_1_0, BUILDER.rq().version);
//     ASSERT_EQ("chunked", BUILDER.rq().GetHeaderVal("transfer-encoding"));
//     const char *str = "L";
//     ASSERT_EQ(std::vector<char>(str, str + strlen(str)), BUILDER.rq().body);
//     ASSERT_EQ(http::RQ_GOOD, BUILDER.rq().status);
// }

// // Valid without body:
// TEST(ValidWithoutBody, 6_SimpleGet_Buffer_50) {
//     if (BuildRequest("requests/rq6.txt", 50) != 0) {
//         FAIL();
//     }
//     ASSERT_EQ(http::HTTP_GET, BUILDER.rq().method);
//     ASSERT_EQ("/", BUILDER.rq().uri);
//     ASSERT_EQ(http::HTTP_1_1, BUILDER.rq().version);
//     ASSERT_EQ("www.example.com", BUILDER.rq().GetHeaderVal("host"));
//     ASSERT_EQ("", BUILDER.rq().GetHeaderVal("content-length"));
//     ASSERT_TRUE(BUILDER.rq().body.empty());
//     ASSERT_EQ(http::RQ_GOOD, BUILDER.rq().status);
// }


// TEST(ValidWithoutBody, 7_GetWithQuery_Buffer_100) {
//     if (BuildRequest("requests/rq7.txt", 100) != 0) {
//         FAIL();
//     }
//     ASSERT_EQ(http::HTTP_GET, BUILDER.rq().method);
//     ASSERT_EQ("/search?q=example", BUILDER.rq().uri);
//     ASSERT_EQ(http::HTTP_1_1, BUILDER.rq().version);
//     ASSERT_EQ("www.search.com", BUILDER.rq().GetHeaderVal("host"));
//     ASSERT_EQ("", BUILDER.rq().GetHeaderVal("content-length"));
//     ASSERT_TRUE(BUILDER.rq().body.empty());
//     ASSERT_EQ(http::RQ_GOOD, BUILDER.rq().status);
// }

// TEST(ValidWithoutBody, 8_GetWithHeaders_Buffer_100) {
//     if (BuildRequest("requests/rq8.txt", 100) != 0) {
//         FAIL();
//     }
//     ASSERT_EQ(http::HTTP_GET, BUILDER.rq().method);
//     ASSERT_EQ("/products", BUILDER.rq().uri);
//     ASSERT_EQ(http::HTTP_1_1, BUILDER.rq().version);
//     ASSERT_EQ("shop.example.com", BUILDER.rq().GetHeaderVal("host"));
//     ASSERT_EQ("application/json", BUILDER.rq().GetHeaderVal("accept"));
//     ASSERT_EQ("CustomClient/1.0", BUILDER.rq().GetHeaderVal("user-agent"));
//     ASSERT_TRUE(BUILDER.rq().body.empty());
//     ASSERT_EQ(http::RQ_GOOD, BUILDER.rq().status);
// }

// TEST(InValidWithoutBody, 9_PostWithHeaders_Buffer_80) {
//     if (BuildRequest("requests/rq9.txt", 80) != 0) {
//         FAIL();
//     }
//     ASSERT_EQ(http::HTTP_POST, BUILDER.rq().method);
//     ASSERT_EQ("/submit", BUILDER.rq().uri);
//     ASSERT_EQ(http::HTTP_1_1, BUILDER.rq().version);
//     ASSERT_EQ("www.example.com", BUILDER.rq().GetHeaderVal("host"));
//     ASSERT_EQ("application/x-www-form-urlencoded", BUILDER.rq().GetHeaderVal("content-type"));
//     ASSERT_EQ("http://www.example.com", BUILDER.rq().GetHeaderVal("referer"));
//     ASSERT_TRUE(BUILDER.rq().body.empty());
//     ASSERT_EQ(http::RQ_BAD, BUILDER.rq().status);
// }

// TEST(ValidWithoutBody, 10_DeleteWithHeaders_Buffer_50) {
//     if (BuildRequest("requests/rq10.txt", 50) != 0) {
//         FAIL();
//     }
//     ASSERT_EQ(http::HTTP_DELETE, BUILDER.rq().method);
//     ASSERT_EQ("/items/123", BUILDER.rq().uri);
//     ASSERT_EQ(http::HTTP_1_0, BUILDER.rq().version);
//     ASSERT_EQ("api.items.com", BUILDER.rq().GetHeaderVal("host"));
//     ASSERT_EQ("Bearer some_token", BUILDER.rq().GetHeaderVal("authorization"));
//     ASSERT_TRUE(BUILDER.rq().body.empty());
//     ASSERT_EQ(http::RQ_GOOD, BUILDER.rq().status);
// }

// TEST(InValidWithoutBody, 11_Incomplete_Method_Buffer_7) {
// //     if (BuildRequest("requests/rq11.txt", 7) != 0) {
//         FAIL();
//     }
//     ASSERT_EQ(http::RQ_BAD, BUILDER.rq().status);
// }

// TEST(InValidWithoutBody, 12_Too_Many_Spaces_Buffer_7) {
// //     if (BuildRequest("requests/rq12.txt", 7) != 0) {
//         FAIL();
//     }
//     ASSERT_EQ(http::HTTP_GET, BUILDER.rq().method);
//     ASSERT_EQ(http::RQ_BAD, BUILDER.rq().status);
// }

// TEST(InValidWithoutBody, 13_No_URI_Buffer_10) {
// //     if (BuildRequest("requests/rq13.txt", 10) != 0) {
//         FAIL();
//     }
//     ASSERT_EQ(http::HTTP_GET, BUILDER.rq().method);
//     ASSERT_TRUE(BUILDER.rq().uri.empty());
//     ASSERT_EQ(http::HTTP_NO_VERSION, BUILDER.rq().version);
//     ASSERT_EQ(http::RQ_BAD, BUILDER.rq().status);
// }

// TEST(InValidWithoutBody, 14_No_Invalid_Version_Buffer_1000) {
// //     if (BuildRequest("requests/rq14.txt", 1000) != 0) {
//         FAIL();
//     }
//     ASSERT_EQ(http::HTTP_GET, BUILDER.rq().method);
//     ASSERT_EQ("/upload", BUILDER.rq().uri);
//     ASSERT_EQ(http::HTTP_NO_VERSION, BUILDER.rq().version);
//     ASSERT_EQ(http::RQ_BAD, BUILDER.rq().status);
// }

// // when testing this and viewing rq15.txt in editor: careful about VS Code setting "Files: Insert Final Newline"
// TEST(InValidWithoutBody, 15_No_CRLF_After_Version_Buffer_10000) {
// //     if (BuildRequest("requests/rq15.txt", 10000) != 0) {
//         FAIL();
//     }
//     ASSERT_EQ(http::HTTP_DELETE, BUILDER.rq().method);
//     ASSERT_EQ("/upload/subfolder1/subfolder2", BUILDER.rq().uri);
//     ASSERT_EQ(http::HTTP_NO_VERSION, BUILDER.rq().version);
//     ASSERT_EQ(http::RQ_BAD, BUILDER.rq().status);
// }

// TEST(InValidWithoutBody, 16_Just_LF_After_Version_Buffer_13) {
// //     if (BuildRequest("requests/rq16.txt", 13) != 0) {
//         FAIL();
//     }
//     ASSERT_EQ(http::HTTP_DELETE, BUILDER.rq().method);
//     ASSERT_EQ("/upload/subfolder1/subfolder2", BUILDER.rq().uri);
//     ASSERT_EQ(http::HTTP_NO_VERSION, BUILDER.rq().version);
//     ASSERT_EQ(http::RQ_BAD, BUILDER.rq().status);
// }

// TEST(InValidWithoutBody, 17_Bad_Header_Key_Buffer_9) {
// //     if (BuildRequest("requests/rq17.txt", 9) != 0) {
//         FAIL();
//     }
//     ASSERT_EQ(http::HTTP_DELETE, BUILDER.rq().method);
//     ASSERT_EQ("/upload/subfolder1/subfolder2", BUILDER.rq().uri);
//     ASSERT_EQ(http::HTTP_2, BUILDER.rq().version);
//     ASSERT_EQ(http::RQ_BAD, BUILDER.rq().status);
// }

// TEST(InValidWithoutBody, 18_Bad_Header_Key_Buffer_1) {
// //     if (BuildRequest("requests/rq18.txt", 1) != 0) {
//         FAIL();
//     }
//     ASSERT_EQ(http::HTTP_DELETE, BUILDER.rq().method);
//     ASSERT_EQ("/upload/subfolder1/subfolder2", BUILDER.rq().uri);
//     ASSERT_EQ(http::HTTP_2, BUILDER.rq().version);
//     ASSERT_EQ(http::RQ_BAD, BUILDER.rq().status);
// }

// TEST(InValidWithoutBody, 19_Bad_Header_Key_Buffer_100) {
// //     if (BuildRequest("requests/rq19.txt", 100) != 0) {
//         FAIL();
//     }
//     ASSERT_EQ(http::HTTP_GET, BUILDER.rq().method);
//     ASSERT_EQ("/", BUILDER.rq().uri);
//     ASSERT_EQ(http::HTTP_1_1, BUILDER.rq().version);
//     ASSERT_TRUE(BUILDER.rq().GetHeaderVal("Host!").empty());
//     ASSERT_EQ(http::RQ_BAD, BUILDER.rq().status);
// }

// TEST(InValidWithoutBody, 20_Bad_Header_Key_Buffer_100) {
// //     if (BuildRequest("requests/rq20.txt", 100) != 0) {
//         FAIL();
//     }
//     ASSERT_EQ(http::HTTP_GET, BUILDER.rq().method);
//     ASSERT_EQ("/", BUILDER.rq().uri);
//     ASSERT_EQ(http::HTTP_1_1, BUILDER.rq().version);
//     ASSERT_TRUE(BUILDER.rq().GetHeaderVal("Host-").empty());
//     ASSERT_EQ(http::RQ_BAD, BUILDER.rq().status);
// }

// TEST(InValidWithoutBody, 21_Bad_Header_Key_Buffer_100) {
// //     if (BuildRequest("requests/rq21.txt", 100) != 0) {
//         FAIL();
//     }
//     ASSERT_EQ(http::HTTP_GET, BUILDER.rq().method);
//     ASSERT_EQ("/", BUILDER.rq().uri);
//     ASSERT_EQ(http::HTTP_1_1, BUILDER.rq().version);
//     ASSERT_TRUE(BUILDER.rq().GetHeaderVal("Host ").empty());
//     ASSERT_EQ(http::RQ_BAD, BUILDER.rq().status);
// }

// TEST(InValidWithoutBody, 22_Missing_Space_Buffer_70) {
// //     if (BuildRequest("requests/rq22.txt", 70) != 0) {
//         FAIL();
//     }
//     ASSERT_EQ(http::HTTP_GET, BUILDER.rq().method);
//     ASSERT_EQ("/", BUILDER.rq().uri);
//     ASSERT_EQ(http::HTTP_1_1, BUILDER.rq().version);
//     ASSERT_TRUE(BUILDER.rq().GetHeaderVal("Host").empty());
//     ASSERT_EQ(http::RQ_BAD, BUILDER.rq().status);
// }

// TEST(InValidWithoutBody, 23_Missing_Header_Value_Buffer_70) {
// //     if (BuildRequest("requests/rq23.txt", 70) != 0) {
//         FAIL();
//     }
//     ASSERT_EQ(http::HTTP_GET, BUILDER.rq().method);
//     ASSERT_EQ("/", BUILDER.rq().uri);
//     ASSERT_EQ(http::HTTP_1_1, BUILDER.rq().version);
//     ASSERT_TRUE(BUILDER.rq().GetHeaderVal("Host").empty());
//     ASSERT_EQ(http::RQ_BAD, BUILDER.rq().status);
// }

// TEST(InValidWithoutBody, 24_No_CRLF_After_Header_Value_Buffer_10) {
// //     if (BuildRequest("requests/rq24.txt", 10) != 0) {
//         FAIL();
//     }
//     ASSERT_EQ(http::HTTP_GET, BUILDER.rq().method);
//     ASSERT_EQ("/", BUILDER.rq().uri);
//     ASSERT_EQ(http::HTTP_1_1, BUILDER.rq().version);
//     ASSERT_TRUE(BUILDER.rq().GetHeaderVal("Host").empty());
//     ASSERT_EQ(http::RQ_BAD, BUILDER.rq().status);
// }

//TEST(InValidWithoutBody, 25_Bad_Header_Value_Buffer_10) {
// TODO: Tests 25-30 check headervalues with invalid characters


//TODO: Tests 31+ check for conflicting headers? multiple headers with same key?


//TODO: Tests 41+ check for invalid body content

int main (int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
