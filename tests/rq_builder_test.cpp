#include <RequestBuilder.h>
#include <gtest/gtest.h>
#include <logger.h>

#include <cstring>
#include <fstream>
#include <iostream>

#include "Request.h"
#include "ResponseCodes.h"
#include "Socket.h"
#include "file_utils.h"
#include "http.h"

#define BODY_14 "Hello, World!!"

#define BODY_1100                                                                                  \
    "Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor "      \
    "invidunt ut labore et dolore magna aliquyam erat, sed diam voluptua. At vero eos et accusam " \
    "et justo duo dolores et ea rebum. Stet clita kasd gubergren, no sea takimata sanctus est "    \
    "Lorem ipsum dolor sit amet. Lorem ipsum dolor sit amet, consetetur sadipscing elitr, sed "    \
    "diam nonumy eirmod tempor invidunt ut labore et dolore magna aliquyam erat, sed diam "        \
    "voluptua. At vero eos et accusam et justo duo dolores et ea rebum. Stet clita kasd "          \
    "gubergren, no sea takimata sanctus est Lorem ipsum dolor sit amet. Lorem ipsum dolor sit "    \
    "amet, consetetur sadipscing elitr, sed diam nonumy eirmod tempor invidunt ut labore et "      \
    "dolore magna aliquyam erat, sed diam voluptua. At vero eos et accusam et justo duo dolores "  \
    "et ea rebum. Stet clita kasd gubergren, no sea takimata sanctus est Lorem ipsum dolor sit "   \
    "amet. Duis autem vel eum iriure dolor in hendrerit in vulputate velit esse molestie "         \
    "consequat, vel illum dolore eu feugiat nulla facilisis at vero eros et accumsan et iusto "    \
    "odio dignissim qui blandit praesent luptat!!!"

#define BODY_1500                                                                                  \
    "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Mauris non enim maximus, efficitur " \
    "dui sed, sagittis massa. Ut id magna justo. Pellentesque in rhoncus risus. Proin pulvinar "   \
    "pulvinar viverra. Etiam nisl nibh, condimentum at gravida non, pulvinar in nulla. Nullam in " \
    "sapien odio. Phasellus fringilla ipsum vel purus fermentum rhoncus. Nulla vitae nibh elit. "  \
    "Vestibulum facilisis orci ac tincidunt laoreet. Nullam quis gravida justo, nec faucibus mi. " \
    "Pellentesque vitae suscipit neque. Sed accumsan, risus at auctor mollis, diam elit "          \
    "efficitur ante, ut venenatis magna erat at metus. Aliquam id turpis maximus, viverra justo "  \
    "ac, tempus nibh. Duis metus ligula, luctus nec lacus quis, aliquam egestas eros. Sed "        \
    "gravida cursus risus, ut facilisis urna condimentum eu. Suspendisse eleifend eleifend "       \
    "ligula eget dignissim. Maecenas ipsum turpis, convallis a purus eu, efficitur fringilla "     \
    "dolor. Ut commodo enim vel leo gravida, vitae efficitur ipsum finibus. Praesent nibh sem, "   \
    "euismod in sagittis iaculis, ultrices a enim. Vestibulum nec orci leo. Vestibulum ac turpis " \
    "ipsum. Phasellus vel est sed ipsum ullamcorper dignissim placerat ornare est. Nullam "        \
    "dignissim finibus enim et faucibus. In hac habitasse platea dictumst. Praesent pharetra "     \
    "dolor in imperdiet sollicitudin. Aenean consequat sapien eget commodo suscipit. Donec "       \
    "convallis est est, sit amet bibendum purus egestas sed. Nulla vel turpis vehicula, lobortis " \
    "dolor id, blandit dolor. Sed dignissim eleifend justo, a accumsan purus tell."

// max body size is set to 1500 in RequestBuilder::MatchServer_()

#define MAX_READ_SZ 1500ul
#define RQ_DIR "test_data/requests/"

char buf[MAX_READ_SZ];

c_api::RecvPackage Recv(std::ifstream& file, size_t read_sz)
{
    c_api::RecvPackage pack;
    file.read(buf, std::max(read_sz, MAX_READ_SZ));
    pack.data_size = file.gcount();
    if (pack.data_size < 0) {
        pack.data = NULL;
        pack.status = c_api::RS_SOCK_ERR;
    } else {
        pack.data = buf;
        pack.status = c_api::RS_OK;
    }
    return pack;
}


// Client Callback context
bool Call(http::RequestBuilder& builder, std::ifstream& file, size_t read_sz)
{
    c_api::RecvPackage pack = Recv(file, read_sz);
    if (pack.status == c_api::RS_SOCK_ERR) {
        std::cerr << "Could not read from client: closing connection..." << std::endl;
        // client_.CloseConnection();
        return false;
    }
    builder.Build(pack.data, pack.data_size);
    return true;
}

int BuildRequest(http::RequestBuilder& builder, const char* rq_path, size_t read_size = 10)
{
    std::string fname = RQ_DIR + std::string(rq_path);
    std::ifstream file(fname);
    if (!file.is_open()) {
        std::cerr << "Could not open Request File: " << rq_path << std::endl;
        return 1;
    }
    while (builder.builder_status() != http::RB_DONE) {
        if (!Call(builder, file, read_size)) {
            break;
        };
    }
    return 0;
}

std::string GetBodyContent_(const http::Request& rq)
{
    if (!rq.has_body()) {
        return "";
    }
    utils::maybe<std::string> body_str;
    body_str = utils::ReadFileToString(rq.body);
    if (!body_str) {
        ADD_FAILURE() << "Error reading body content from file";
    }
    return *body_str;
}

class DummyCb : public http::IChooseServerCb {
  public:
    DummyCb() {}
    http::ChosenServerParams Call(const http::Request&)
    {
        http::ChosenServerParams params;
        params.max_body_size = 1500;
        return params;
    }
};

http::RequestBuilder CreateBuilder()
{
    return http::RequestBuilder(utils::unique_ptr<http::IChooseServerCb>(new DummyCb()));
}

TEST(ValidWithBody, 1_Bodylen_14)
{
    http::RequestBuilder builder = CreateBuilder();
    if (BuildRequest(builder, "rq1.txt", 50) != 0) {
        FAIL();
    }

    EXPECT_EQ(http::HTTP_POST, builder.rq().method);
    EXPECT_EQ("/", builder.rq().rqTarget.ToStr());
    EXPECT_EQ(http::HTTP_1_1, builder.rq().version);
    EXPECT_EQ("www.example.com", builder.rq().GetHeaderVal("host").value());
    EXPECT_TRUE(builder.rq().has_body());
    EXPECT_EQ("14", builder.rq().GetHeaderVal("content-length").value());
    EXPECT_EQ(std::string(BODY_14), GetBodyContent_(builder.rq()));
    EXPECT_EQ(http::HTTP_OK, builder.rq().status);
}

TEST(ValidWithBody, 2_One_Chunk_1100)
{
    http::RequestBuilder builder = CreateBuilder();
    if (BuildRequest(builder, "rq2.txt", 10) != 0) {
        FAIL();
    }
    EXPECT_EQ(http::HTTP_POST, builder.rq().method);
    EXPECT_EQ("/upload", builder.rq().rqTarget.ToStr());
    EXPECT_EQ(http::HTTP_1_1, builder.rq().version);
    EXPECT_EQ("chunked", builder.rq().GetHeaderVal("transfer-encoding").value());
    EXPECT_EQ(std::string(BODY_1100), GetBodyContent_(builder.rq()));
    EXPECT_EQ(http::HTTP_OK, builder.rq().status);
}

TEST(ValidWithBody, 3_One_Chunk_1100)
{
    http::RequestBuilder builder = CreateBuilder();
    if (BuildRequest(builder, "rq2.txt", 9) != 0) {
        FAIL();
    }
    EXPECT_EQ(http::HTTP_POST, builder.rq().method);
    EXPECT_EQ("/upload", builder.rq().rqTarget.ToStr());
    EXPECT_EQ(http::HTTP_1_1, builder.rq().version);
    EXPECT_EQ("chunked", builder.rq().GetHeaderVal("transfer-encoding").value());
    EXPECT_EQ(std::string(BODY_1100), GetBodyContent_(builder.rq()));
    EXPECT_EQ(http::HTTP_OK, builder.rq().status);
}

TEST(ValidWithBody, 4_Bodylen_1)
{
    http::RequestBuilder builder = CreateBuilder();
    if (BuildRequest(builder, "rq4.txt", 50) != 0) {
        FAIL();
    }
    EXPECT_EQ(http::HTTP_POST, builder.rq().method);
    EXPECT_EQ("/", builder.rq().rqTarget.ToStr());
    EXPECT_EQ(http::HTTP_1_1, builder.rq().version);
    EXPECT_EQ("www.example.com", builder.rq().GetHeaderVal("host").value());
    EXPECT_EQ("1", builder.rq().GetHeaderVal("content-length").value());
    EXPECT_EQ("a", GetBodyContent_(builder.rq()));
    EXPECT_EQ(http::HTTP_OK, builder.rq().status);
}

TEST(ValidWithBody, 5_Chunked_1)
{
    http::RequestBuilder builder = CreateBuilder();
    if (BuildRequest(builder, "rq5.txt", 50) != 0) {
        FAIL();
    }
    EXPECT_EQ(http::HTTP_POST, builder.rq().method);
    EXPECT_EQ("/upload", builder.rq().rqTarget.ToStr());
    EXPECT_EQ(http::HTTP_1_0, builder.rq().version);
    EXPECT_EQ("chunked", builder.rq().GetHeaderVal("transfer-encoding").value());
    EXPECT_EQ("L", GetBodyContent_(builder.rq()));
    EXPECT_EQ(http::HTTP_OK, builder.rq().status);
}

// Valid without body:
TEST(ValidWithoutBody, 6_SimpleGet)
{
    http::RequestBuilder builder = CreateBuilder();
    if (BuildRequest(builder, "rq6.txt", 50) != 0) {
        FAIL();
    }
    EXPECT_EQ(http::HTTP_GET, builder.rq().method);
    EXPECT_EQ("/", builder.rq().rqTarget.ToStr());
    EXPECT_EQ(http::HTTP_1_1, builder.rq().version);
    EXPECT_EQ("www.example.com", builder.rq().GetHeaderVal("host").value());
    EXPECT_FALSE(builder.rq().GetHeaderVal("content-length").ok());
    EXPECT_FALSE(builder.rq().has_body());
    EXPECT_EQ(http::HTTP_OK, builder.rq().status);
}

TEST(ValidWithoutBody, 7_GetWithQuery)
{
    http::RequestBuilder builder = CreateBuilder();
    if (BuildRequest(builder, "rq7.txt", 100) != 0) {
        FAIL();
    }
    EXPECT_EQ(http::HTTP_GET, builder.rq().method);
    EXPECT_EQ("/search?q=example", builder.rq().rqTarget.ToStr());
    EXPECT_EQ(http::HTTP_1_1, builder.rq().version);
    EXPECT_EQ("www.search.com", builder.rq().GetHeaderVal("host").value());
    EXPECT_FALSE(builder.rq().GetHeaderVal("content-length").ok());
    EXPECT_FALSE(builder.rq().has_body());
    EXPECT_EQ(http::HTTP_OK, builder.rq().status);
}

TEST(ValidWithoutBody, 8_GetWithHeaders)
{
    http::RequestBuilder builder = CreateBuilder();
    if (BuildRequest(builder, "rq8.txt", 100) != 0) {
        FAIL();
    }
    EXPECT_EQ(http::HTTP_GET, builder.rq().method);
    EXPECT_EQ("/products", builder.rq().rqTarget.ToStr());
    EXPECT_EQ(http::HTTP_1_1, builder.rq().version);
    EXPECT_EQ("shop.example.com", builder.rq().GetHeaderVal("host").value());
    EXPECT_EQ("application/json", builder.rq().GetHeaderVal("accept").value());
    EXPECT_EQ("CustomClient/1.0", builder.rq().GetHeaderVal("user-agent").value());
    EXPECT_FALSE(builder.rq().has_body());
    EXPECT_EQ(http::HTTP_OK, builder.rq().status);
}

// POST without content-length or chunked: BAD_REQUEST?
TEST(InValidWithoutBody, 9_PostWithHeaders)
{
    http::RequestBuilder builder = CreateBuilder();
    if (BuildRequest(builder, "rq9.txt", 80) != 0) {
        FAIL();
    }
    EXPECT_EQ(http::HTTP_POST, builder.rq().method);
    EXPECT_EQ("/submit", builder.rq().rqTarget.ToStr());
    EXPECT_EQ(http::HTTP_1_1, builder.rq().version);
    EXPECT_EQ("www.example.com", builder.rq().GetHeaderVal("host").value());
    EXPECT_EQ("application/x-www-form-urlencoded",
              builder.rq().GetHeaderVal("content-type").value());
    EXPECT_EQ("http://www.example.com", builder.rq().GetHeaderVal("referer").value());
    EXPECT_FALSE(builder.rq().has_body());
    EXPECT_EQ(http::HTTP_LENGTH_REQUIRED, builder.rq().status);
}

TEST(ValidWithoutBody, 10_DeleteWithHeaders)
{
    http::RequestBuilder builder = CreateBuilder();
    if (BuildRequest(builder, "rq10.txt", 50) != 0) {
        FAIL();
    }
    EXPECT_EQ(http::HTTP_DELETE, builder.rq().method);
    EXPECT_EQ("/items/123", builder.rq().rqTarget.ToStr());
    EXPECT_EQ(http::HTTP_1_0, builder.rq().version);
    EXPECT_EQ("api.items.com", builder.rq().GetHeaderVal("host").value());
    EXPECT_EQ("Bearer_some_token", builder.rq().GetHeaderVal("authorization").value());
    EXPECT_FALSE(builder.rq().has_body());
    EXPECT_EQ(http::HTTP_OK, builder.rq().status);
}

TEST(InValidWithoutBody, 11_Incomplete_Method)
{
    http::RequestBuilder builder = CreateBuilder();
    if (BuildRequest(builder, "rq11.txt", 7) != 0) {
        FAIL();
    }
    EXPECT_EQ(http::HTTP_BAD_REQUEST, builder.rq().status);
}

TEST(InValidWithoutBody, 12_Too_Many_Spaces)
{
    http::RequestBuilder builder = CreateBuilder();
    if (BuildRequest(builder, "rq12.txt", 7) != 0) {
        FAIL();
    }
    EXPECT_EQ(http::HTTP_BAD_REQUEST, builder.rq().status);
}

TEST(InValidWithoutBody, 13_No_URI)
{
    http::RequestBuilder builder = CreateBuilder();
    if (BuildRequest(builder, "rq13.txt", 10) != 0) {
        FAIL();
    }
    EXPECT_TRUE(builder.rq().rqTarget.ToStr().empty());
    EXPECT_EQ(http::HTTP_NO_VERSION, builder.rq().version);
    EXPECT_EQ(http::HTTP_BAD_REQUEST, builder.rq().status);
}

TEST(InValidWithoutBody, 14_Invalid_Version)
{
    http::RequestBuilder builder = CreateBuilder();
    if (BuildRequest(builder, "rq14.txt", 1000) != 0) {
        FAIL();
    }
    EXPECT_EQ(http::HTTP_GET, builder.rq().method);
    EXPECT_EQ("/upload", builder.rq().rqTarget.ToStr());
    EXPECT_EQ(http::HTTP_NO_VERSION, builder.rq().version);
    EXPECT_EQ(http::HTTP_BAD_REQUEST, builder.rq().status);
}

// when testing this and viewing rq15.txt in editor: careful about VS Code setting "Files: Insert
// Final Newline"
TEST(InValidWithoutBody, 15_No_CRLF_After_Version)
{
    http::RequestBuilder builder = CreateBuilder();
    if (BuildRequest(builder, "rq15.txt", 10000) != 0) {
        FAIL();
    }
    EXPECT_EQ(http::HTTP_NO_VERSION, builder.rq().version);
    EXPECT_EQ(http::HTTP_BAD_REQUEST, builder.rq().status);
}

TEST(InValidWithoutBody, 16_Just_LF_After_Version)
{
    http::RequestBuilder builder = CreateBuilder();
    if (BuildRequest(builder, "rq16.txt", 13) != 0) {
        FAIL();
    }
    EXPECT_EQ(http::HTTP_NO_VERSION, builder.rq().version);
    EXPECT_EQ(http::HTTP_BAD_REQUEST, builder.rq().status);
}

TEST(InValidWithoutBody, 17_Bad_Header_Key)
{
    http::RequestBuilder builder = CreateBuilder();
    if (BuildRequest(builder, "rq17.txt", 9) != 0) {
        FAIL();
    }
    EXPECT_EQ(http::HTTP_DELETE, builder.rq().method);
    EXPECT_EQ("/upload/subfolder1/subfolder2", builder.rq().rqTarget.ToStr());
    EXPECT_EQ(http::HTTP_1_0, builder.rq().version);
    EXPECT_EQ(http::HTTP_BAD_REQUEST, builder.rq().status);
}

TEST(InValidWithoutBody, 18_Bad_Header_Key)
{
    http::RequestBuilder builder = CreateBuilder();
    if (BuildRequest(builder, "rq18.txt", 1) != 0) {
        FAIL();
    }
    EXPECT_EQ(http::HTTP_DELETE, builder.rq().method);
    EXPECT_EQ("/upload/subfolder1/subfolder2", builder.rq().rqTarget.ToStr());
    EXPECT_EQ(http::HTTP_1_0, builder.rq().version);
    EXPECT_EQ(http::HTTP_BAD_REQUEST, builder.rq().status);
}

TEST(InValidWithoutBody, 19_Bad_Header_Key)
{
    http::RequestBuilder builder = CreateBuilder();
    if (BuildRequest(builder, "rq19.txt", 100) != 0) {
        FAIL();
    }
    EXPECT_EQ(http::HTTP_GET, builder.rq().method);
    EXPECT_EQ("/", builder.rq().rqTarget.ToStr());
    EXPECT_EQ(http::HTTP_1_1, builder.rq().version);
    EXPECT_EQ(http::HTTP_BAD_REQUEST, builder.rq().status);
}

TEST(InValidWithoutBody, 20_Bad_Header_Key)
{
    http::RequestBuilder builder = CreateBuilder();
    if (BuildRequest(builder, "rq20.txt", 100) != 0) {
        FAIL();
    }
    EXPECT_EQ(http::HTTP_GET, builder.rq().method);
    EXPECT_EQ("/", builder.rq().rqTarget.ToStr());
    EXPECT_EQ(http::HTTP_1_1, builder.rq().version);
    EXPECT_EQ(http::HTTP_BAD_REQUEST, builder.rq().status);
}

TEST(InValidWithoutBody, 21_Bad_Header_Key)
{
    http::RequestBuilder builder = CreateBuilder();
    if (BuildRequest(builder, "rq21.txt", 100) != 0) {
        FAIL();
    }
    EXPECT_EQ(http::HTTP_GET, builder.rq().method);
    EXPECT_EQ("/", builder.rq().rqTarget.ToStr());
    EXPECT_EQ(http::HTTP_1_1, builder.rq().version);
    EXPECT_EQ(http::HTTP_BAD_REQUEST, builder.rq().status);
}

TEST(InValidWithoutBody, 24_No_CRLF_After_Header_Value)
{
    http::RequestBuilder builder = CreateBuilder();
    if (BuildRequest(builder, "rq24.txt", 10) != 0) {
        FAIL();
    }
    EXPECT_EQ(http::HTTP_GET, builder.rq().method);
    EXPECT_EQ("/", builder.rq().rqTarget.ToStr());
    EXPECT_EQ(http::HTTP_1_1, builder.rq().version);
    EXPECT_FALSE(builder.rq().GetHeaderVal("host").ok());
    EXPECT_EQ(http::HTTP_BAD_REQUEST, builder.rq().status);
}

// TEST(InValidWithoutBody, 25_Bad_Header_Value_10) {
//  TODO: Tests 25-30 check headervalues with invalid characters


// TODO: Tests 31+ check for conflicting headers? multiple headers with same key?


// TODO: Tests 41+ check for invalid body content

TEST(InValidWithBody, 50_Bad_Chunk_size_has_plus)
{
    http::RequestBuilder builder = CreateBuilder();
    if (BuildRequest(builder, "rq50.txt", 1000) != 0) {
        FAIL();
    }
    EXPECT_EQ(http::HTTP_POST, builder.rq().method);
    EXPECT_EQ("/", builder.rq().rqTarget.ToStr());
    EXPECT_EQ(http::HTTP_1_1, builder.rq().version);
    EXPECT_EQ("chunked", builder.rq().GetHeaderVal("Transfer-Encoding").value());
    EXPECT_EQ(http::HTTP_BAD_REQUEST, builder.rq().status);
}

TEST(InValidWithBody, 51_Bad_Chunk_size_has_minus)
{
    http::RequestBuilder builder = CreateBuilder();
    if (BuildRequest(builder, "rq51.txt", 1000) != 0) {
        FAIL();
    }
    EXPECT_EQ(http::HTTP_POST, builder.rq().method);
    EXPECT_EQ("/", builder.rq().rqTarget.ToStr());
    EXPECT_EQ(http::HTTP_1_1, builder.rq().version);
    EXPECT_EQ("chunked", builder.rq().GetHeaderVal("Transfer-Encoding").value());
    EXPECT_EQ(http::HTTP_BAD_REQUEST, builder.rq().status);
}

TEST(InValidWithBody, 52_Bad_Chunk_size_has_leading_spaces)
{
    http::RequestBuilder builder = CreateBuilder();
    if (BuildRequest(builder, "rq52.txt", 1000) != 0) {
        FAIL();
    }
    EXPECT_EQ(http::HTTP_POST, builder.rq().method);
    EXPECT_EQ("/", builder.rq().rqTarget.ToStr());
    EXPECT_EQ(http::HTTP_1_1, builder.rq().version);
    EXPECT_EQ("chunked", builder.rq().GetHeaderVal("Transfer-Encoding").value());
    EXPECT_EQ(http::HTTP_BAD_REQUEST, builder.rq().status);
}

TEST(InValidWithBody, 53_Bad_Chunk_size_has_trailing_spaces)
{
    http::RequestBuilder builder = CreateBuilder();
    if (BuildRequest(builder, "rq51.txt", 1000) != 0) {
        FAIL();
    }
    EXPECT_EQ(http::HTTP_POST, builder.rq().method);
    EXPECT_EQ("/", builder.rq().rqTarget.ToStr());
    EXPECT_EQ(http::HTTP_1_1, builder.rq().version);
    EXPECT_EQ("chunked", builder.rq().GetHeaderVal("Transfer-Encoding").value());
    EXPECT_EQ(http::HTTP_BAD_REQUEST, builder.rq().status);
}

TEST(MaxBodySize, 55_Body_too_large)
{
    http::RequestBuilder builder = CreateBuilder();
    if (BuildRequest(builder, "rq55.txt", 1000) != 0) {
        FAIL();
    }
    EXPECT_EQ("1501", builder.rq().GetHeaderVal("content-length").value());
    EXPECT_EQ(http::HTTP_PAYLOAD_TOO_LARGE, builder.rq().status);
}

TEST(LengthLimits, 60_Uri_too_long)
{
    http::RequestBuilder builder = CreateBuilder();
    if (BuildRequest(builder, "rq60.txt", 1000) != 0) {
        FAIL();
    }
    if (RQ_LINE_LEN_LIMIT <= 8192) {
        EXPECT_EQ(http::HTTP_BAD_REQUEST, builder.rq().status);
    } else {
        EXPECT_EQ(http::HTTP_OK, builder.rq().status);
    }
}

TEST(LengthLimits, 61_Header_Too_Long)
{
    http::RequestBuilder builder = CreateBuilder();
    if (BuildRequest(builder, "rq61.txt", 1000) != 0) {
        FAIL();
    }
    if (RQ_LINE_LEN_LIMIT <= 8192) {
        EXPECT_EQ(http::HTTP_BAD_REQUEST, builder.rq().status);
    } else {
        EXPECT_EQ(http::HTTP_OK, builder.rq().status);
    }
}

TEST(LengthLimits, 62_Header_Section_Too_Large)
{
    http::RequestBuilder builder = CreateBuilder();
    if (BuildRequest(builder, "rq62.txt", 1000) != 0) {
        FAIL();
    }
    if (RQ_HEADER_SECTION_LIMIT <= 32768) {
        EXPECT_EQ(http::HTTP_REQUEST_HEADER_FIELDS_TOO_LARGE, builder.rq().status);
    } else {
        EXPECT_EQ(http::HTTP_OK, builder.rq().status);
    }
}

TEST(LengthLimits, 65_Too_Many_Headers)
{
    http::RequestBuilder builder = CreateBuilder();
    if (BuildRequest(builder, "rq65.txt", 1000) != 0) {
        FAIL();
    }
    if (RQ_MAX_HEADER_COUNT <= 100) {
        EXPECT_EQ(http::HTTP_REQUEST_HEADER_FIELDS_TOO_LARGE, builder.rq().status);
    } else {
        EXPECT_EQ(http::HTTP_OK, builder.rq().status);
    }
}


TEST(BadMethod, 70_Bad_Method)
{
    http::RequestBuilder builder = CreateBuilder();
    if (BuildRequest(builder, "rq70.txt", 1000) != 0) {
        FAIL();
    }
    EXPECT_EQ(http::HTTP_BAD_REQUEST, builder.rq().status);
}

TEST(BadMethod, 76_Unsupported_Method)
{
    http::RequestBuilder builder = CreateBuilder();
    if (BuildRequest(builder, "rq76.txt", 1000) != 0) {
        FAIL();
    }
    EXPECT_EQ(http::HTTP_NOT_IMPLEMENTED, builder.rq().status);
}
