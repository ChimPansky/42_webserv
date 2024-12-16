#include "RequestBuilder.h"

#include <logger.h>
#include <numeric_utils.h>
#include <unistd.h>

#include <algorithm>
#include <cstring>
#include <iterator>
#include <stdexcept>
#include <string>
#include <vector>

#include "Request.h"
#include "ResponseCodes.h"
#include "SyntaxChecker.h"
#include "http.h"
#include "str_utils.h"
#include "unique_ptr.h"

namespace http {

RequestBuilder::RequestBuilder(utils::unique_ptr<IChooseServerCb> choose_server_cb)
    : builder_status_(RB_BUILDING), build_state_(BS_RQ_LINE), choose_server_cb_(choose_server_cb),
      header_count_(0), header_section_size_(0)
{
    if (!choose_server_cb_) {
        throw std::logic_error("No Choose Server Callback specified");
    }
}

RequestBuilder::BodyBuilder::BodyBuilder()
    : chunked(false), body_idx(0), remaining_length(0), max_body_size(0)
{}

void RequestBuilder::PrepareToRecvData(size_t recv_size)
{
    parser_.PrepareToRecvData(recv_size);
}

void RequestBuilder::AdjustBufferSize(size_t bytes_recvd)
{
    parser_.AdjustBufferSize(bytes_recvd);
}

bool RequestBuilder::CanBuild_()
{
    if (build_state_ == BS_BAD_REQUEST || build_state_ == BS_END) {
        return false;
    }
    if (IsParsingState_(build_state_) && parser_.EndOfBuffer()) {
        builder_status_ = http::RB_NEED_DATA_FROM_CLIENT;
        return false;
    }
    return true;
}

// TODO: rm bytes_recvd
void RequestBuilder::Build(size_t bytes_recvd)
{
    LOG(DEBUG) << "RequestBuilder::Build";
    // client session will be killed earlier, so dead code, rm
    if (parser_.EndOfBuffer() && bytes_recvd == 0) {
        LOG(INFO) << "Ran out of data while Request incomplete.";
        rq_.status = HTTP_BAD_REQUEST;
        builder_status_ = RB_DONE;
        return;
    }
    while (CanBuild_()) {
        switch (build_state_) {
            case BS_RQ_LINE: build_state_ = BuildFirstLine_(); break;
            case BS_HEADER_FIELDS: build_state_ = BuildHeaderField_(); break;
            case BS_MATCH_SERVER: build_state_ = MatchServer_(); break;
            case BS_CHECK_HEADERS: build_state_ = CheckHeaders_(); break;
            case BS_PREPARE_TO_READ_BODY: build_state_ = PrepareBody_(); break;
            case BS_BODY_REGULAR: build_state_ = BuildBodyRegular_(); break;
            case BS_BODY_CHUNK_SIZE: build_state_ = BuildBodyChunkSize_(); break;
            case BS_BODY_CHUNK_CONTENT: build_state_ = BuildBodyChunkContent_(); break;
            case BS_BAD_REQUEST: break;
            case BS_END: break;
        }
    }
    if (build_state_ == BS_END || build_state_ == BS_BAD_REQUEST) {
        builder_status_ = RB_DONE;
    }
}

RqBuilderStatus RequestBuilder::builder_status() const
{
    return builder_status_;
}

const Request& RequestBuilder::rq() const
{
    return rq_;
}

std::vector<char>& RequestBuilder::buf()
{
    return parser_.buf();
}

RequestBuilder::BuildState RequestBuilder::BuildFirstLine_()
{
    LOG(DEBUG) << "BuildFirstLine_";

    switch (TryToExtractLine_()) {
        case EXTRACTION_SUCCESS: break;
        case EXTRACTION_CRLF_NOT_FOUND: return BS_RQ_LINE;
        case EXTRACTION_TOO_LONG: return SetStatusAndExitBuilder_(HTTP_URI_TOO_LONG);
        default: {
            LOG(INFO) << "Request Line Syntax Error";
            return SetStatusAndExitBuilder_(HTTP_BAD_REQUEST);
        }
    }
    header_section_size_ += extraction_.size();
    // todo for robustness: if very first line of request empty -> ignore and continue
    std::stringstream ss(extraction_);
    std::string raw_method, raw_rq_target, raw_version;
    std::getline(ss, raw_method, ' ');
    if (ss.eof() || ss.fail()) {
        LOG(INFO) << "Request Line unexpected EOL";
        return SetStatusAndExitBuilder_(HTTP_BAD_REQUEST);
    }
    std::getline(ss, raw_rq_target, ' ');
    if (ss.eof() || ss.fail()) {
        LOG(INFO) << "Request Line unexpected EOL";
        return SetStatusAndExitBuilder_(HTTP_BAD_REQUEST);
    }
    std::getline(ss, raw_version);
    if (!ss.eof() || ss.fail()) {
        LOG(INFO) << "Request Line unexpected EOL";
        return SetStatusAndExitBuilder_(HTTP_BAD_REQUEST);
    }
    ResponseCode rc = TrySetMethod_(raw_method);
    if (rc != http::HTTP_OK) {
        LOG(INFO) << "Cannot determine method";
        return SetStatusAndExitBuilder_(rc);
    }
    rc = TrySetRqTarget_(raw_rq_target);
    if (rc != http::HTTP_OK) {
        LOG(INFO) << "Cannot determine Request-Target";
        return SetStatusAndExitBuilder_(rc);
    }
    rc = TrySetVersion_(raw_version);
    if (rc != http::HTTP_OK) {
        LOG(INFO) << "Cannot determine Version";
        return SetStatusAndExitBuilder_(rc);
    }
    return BS_HEADER_FIELDS;
}

ResponseCode RequestBuilder::TrySetMethod_(const std::string& raw_method)
{
    if (!SyntaxChecker::IsValidMethodName(raw_method)) {
        LOG(INFO) << "Invalid Method name: " << raw_method;
        return HTTP_BAD_REQUEST;
    };
    std::pair<bool, Method> converted_method = HttpMethodFromStr(raw_method);
    if (!converted_method.first) {
        LOG(INFO) << "Method not implemented: " << raw_method;
        return HTTP_NOT_IMPLEMENTED;
    }
    rq_.method = converted_method.second;
    return HTTP_OK;
}

ResponseCode RequestBuilder::TrySetRqTarget_(const std::string& raw_rq_target)
{
    rq_.rqTarget = raw_rq_target;
    if (rq_.rqTarget.validity_state() & RqTarget::RQ_TARGET_TOO_LONG) {
        LOG(INFO) << "Request Target too long: " << raw_rq_target.size();
        return HTTP_URI_TOO_LONG;
    }
    if (!rq_.rqTarget.Good()) {
        LOG(INFO) << "Invalid Request Target: " << raw_rq_target;
        return HTTP_BAD_REQUEST;
    }
    return HTTP_OK;
}

ResponseCode RequestBuilder::TrySetVersion_(const std::string& raw_version)
{
    if (!SyntaxChecker::IsValidVersionName(raw_version)) {
        LOG(INFO) << "Invalid Version name: " << raw_version;
        return HTTP_BAD_REQUEST;
    };
    std::pair<bool, Version> converted_version = HttpVersionFromStr(raw_version);
    if (!converted_version.first) {
        LOG(INFO) << "Version not implemented: " << raw_version;
        return HTTP_BAD_REQUEST;
    }
    rq_.version = converted_version.second;
    return HTTP_OK;
}

// https://www.rfc-editor.org/rfc/rfc9110#name-field-lines-and-combined-fi
// When a field name is only present once in a section, the combined "field value" for that field
// consists of the corresponding field line value. When a field name is repeated within a section,
// its combined field value consists of the list of corresponding field line values within that
// section, concatenated in order, with each field line value separated by a comma.

// For example, this section:

// Example-Field: Foo, Bar
// Example-Field: Baz
// contains two field lines, both with the field name "Example-Field". The first field line has a
// field line value of "Foo, Bar", while the second field line value is "Baz". The field value for
// "Example-Field" is the list "Foo, Bar, Baz".

RequestBuilder::BuildState RequestBuilder::BuildHeaderField_()
{
    LOG(DEBUG) << "BuildHeaderField_";
    switch (TryToExtractLine_()) {
        case EXTRACTION_SUCCESS: break;
        case EXTRACTION_CRLF_NOT_FOUND: return BS_HEADER_FIELDS;
        default: {
            LOG(INFO) << "Header Field Syntax Error";
            return SetStatusAndExitBuilder_(HTTP_BAD_REQUEST);
        }
    }
    header_section_size_ += extraction_.size();
    if (header_section_size_ > RQ_HEADER_SECTION_LIMIT) {
        LOG(INFO) << "Header Section too large";
        return SetStatusAndExitBuilder_(HTTP_REQUEST_HEADER_FIELDS_TOO_LARGE);
    }
    if (extraction_.empty()) {  // empty line -> end of headers
        return BS_MATCH_SERVER;
    }
    std::stringstream ss(extraction_);
    std::string header_key, header_val;
    std::getline(ss, header_key, ':');
    utils::EatSpacesAndHTabs(ss);
    if (ss.eof()) {
        LOG(INFO) << "unexpected EOL after header key";
        return SetStatusAndExitBuilder_(HTTP_BAD_REQUEST);
    }
    std::getline(ss, header_val, '\r');
    if (!ss.eof()) {
        LOG(INFO) << "Extra characters after header val";
        return SetStatusAndExitBuilder_(HTTP_BAD_REQUEST);
    }
    ResponseCode rc = InsertHeaderField_(header_key, header_val);
    if (rc != http::HTTP_OK) {
        LOG(INFO) << "Could not insert header Field";
        return SetStatusAndExitBuilder_(rc);
    }
    return BS_HEADER_FIELDS;
}

RequestBuilder::BuildState RequestBuilder::MatchServer_()
{
    LOG(DEBUG) << "MatchServer_";
    body_builder_.max_body_size = choose_server_cb_->Call(rq_).max_body_size;
    return BS_CHECK_HEADERS;
}

RequestBuilder::BuildState RequestBuilder::CheckHeaders_()
{
    ResponseCode rc = ValidateHeadersSyntax_();
    if (rc != http::HTTP_OK) {
        return SetStatusAndExitBuilder_(rc);
    }
    rc = InterpretHeaders_();
    if (rc != http::HTTP_OK) {
        return SetStatusAndExitBuilder_(rc);
    }
    if (rq_.has_body) {
        return BS_PREPARE_TO_READ_BODY;
    }
    return BS_END;
}

ResponseCode RequestBuilder::ValidateHeadersSyntax_()
{
    LOG(DEBUG) << "ValidateHeadersSyntax_";
    for (std::map<std::string, std::string>::const_iterator it = rq_.headers.begin();
         it != rq_.headers.end(); ++it) {
        if (!SyntaxChecker::IsValidHeaderKeyName(it->first)) {
            LOG(INFO) << "Header Key Syntax Error";
            return HTTP_BAD_REQUEST;
        }
        if (!SyntaxChecker::IsValidHeaderValueName(it->second)) {
            LOG(INFO) << "Header Value Syntax Error";
            return HTTP_BAD_REQUEST;
        }
    }
    // additional semantic checks...
    return HTTP_OK;
}

// todo: do we need to differentiate btw HTTP/1.0 and HTTP/1.1?
ResponseCode RequestBuilder::InterpretHeaders_()
{
    LOG(DEBUG) << "InterpretHeaders_";

    std::pair<bool, std::string> host = rq_.GetHeaderVal("host");
    if (!host.first) {
        LOG(INFO) << "Host header missing";
        return HTTP_BAD_REQUEST;
    }
    std::pair<bool, std::string> content_length = rq_.GetHeaderVal("content-length");
    std::pair<bool, std::string> transfer_encoding = rq_.GetHeaderVal("transfer-encoding");
    if (content_length.first && transfer_encoding.first) {
        LOG(INFO) << "Both Content-Length and Transfer-Encoding headers present";
        return HTTP_BAD_REQUEST;
    }
    if (transfer_encoding.first && transfer_encoding.second == "chunked") {
        rq_.has_body = true;
        body_builder_.chunked = true;
    }
    if (content_length.first) {
        rq_.has_body = true;
        std::pair<bool, size_t> content_length_num =
            utils::StrToNumericNoThrow<size_t>(content_length.second);
        LOG(DEBUG) << "InterpretHeaders_() Max_body_size: " << body_builder_.max_body_size;
        if (content_length_num.first) {
            if (content_length_num.second > body_builder_.max_body_size) {
                LOG(INFO) << "Content-Length too large";
                return HTTP_PAYLOAD_TOO_LARGE;
            }
            body_builder_.remaining_length = content_length_num.second;
        } else {
            LOG(INFO) << "Content-Length not a number";
            return HTTP_BAD_REQUEST;
        }
    }
    if (rq_.method == HTTP_POST && !content_length.first && !transfer_encoding.first) {
        LOG(INFO) << "POST request without Content-Length or Transfer-Encoding";
        return HTTP_LENGTH_REQUIRED;
    }
    // additional semantic checks...
    return HTTP_OK;
}

RequestBuilder::BuildState RequestBuilder::PrepareBody_()
{
    if (!utils::CreateAndOpenTmpFileToStream(body_builder_.body_stream, rq_.body)) {
        LOG(ERROR) << "Failed to create temporary file.";
        return SetStatusAndExitBuilder_(HTTP_INTERNAL_SERVER_ERROR);
    }
    if (body_builder_.chunked) {
        return BS_BODY_CHUNK_SIZE;
    } else {
        if (body_builder_.remaining_length > body_builder_.max_body_size) {
            LOG(INFO) << "Content-Length too large";
            return SetStatusAndExitBuilder_(HTTP_PAYLOAD_TOO_LARGE);
        }
        return BS_BODY_REGULAR;
    }
    return BS_END;
}

RequestBuilder::BuildState RequestBuilder::BuildBodyRegular_()
{
    size_t copy_size = std::min(body_builder_.remaining_length, parser_.RemainingLength());
    const char* begin = parser_.buf().data() + body_builder_.body_idx;
    const char* end = begin + copy_size;
    std::copy(begin, end, std::ostream_iterator<char>(body_builder_.body_stream));
    body_builder_.body_idx += copy_size;
    body_builder_.remaining_length -= copy_size;
    parser_.Advance(copy_size);
    if (body_builder_.remaining_length > 0) {
        return BS_BODY_REGULAR;
    }
    body_builder_.body_stream.close();
    return BS_END;
}

// https://datatracker.ietf.org/doc/html/rfc2616#section-3.5
RequestBuilder::BuildState RequestBuilder::BuildBodyChunkSize_()
{
    LOG(DEBUG) << "BuildBodyChunkSize_";

    switch (TryToExtractLine_()) {
        case EXTRACTION_SUCCESS: break;
        case EXTRACTION_CRLF_NOT_FOUND: return BS_BODY_CHUNK_SIZE;
        default: return SetStatusAndExitBuilder_(HTTP_BAD_REQUEST);
    }
    std::pair<bool, size_t> chunk_size = utils::HexToUnsignedNumericNoThrow<size_t>(extraction_);
    if (!chunk_size.first) {
        LOG(INFO) << "Chunk Size Syntax Error";
        return SetStatusAndExitBuilder_(HTTP_BAD_REQUEST);
    }
    if (chunk_size.second == 0) {
        body_builder_.body_stream.close();
        return BS_END;
    }
    body_builder_.body_idx += chunk_size.second;
    body_builder_.remaining_length = chunk_size.second;
    if (body_builder_.body_idx > body_builder_.max_body_size) {
        LOG(INFO) << "Chunked content too large";
        return SetStatusAndExitBuilder_(HTTP_PAYLOAD_TOO_LARGE);
    }
    return BS_BODY_CHUNK_CONTENT;
}

RequestBuilder::BuildState RequestBuilder::BuildBodyChunkContent_()
{
    LOG(DEBUG) << "BuildBodyChunkContent_";
    switch (TryToExtractBodyContent_()) {
        case EXTRACTION_SUCCESS: break;
        case EXTRACTION_TOO_LONG: return SetStatusAndExitBuilder_(HTTP_BAD_REQUEST);
        default: return BS_BODY_CHUNK_CONTENT;
    }
    std::copy(extraction_.begin(), extraction_.end(),
              std::ostream_iterator<char>(body_builder_.body_stream));
    return BS_BODY_CHUNK_SIZE;
}

RequestBuilder::ExtractionResult RequestBuilder::TryToExtractLine_()
{
    while (!parser_.EndOfBuffer()) {
        if (parser_.ElementLen() > RQ_LINE_LEN_LIMIT) {
            LOG(INFO) << "Line too long";
            return EXTRACTION_TOO_LONG;
        }
        if (parser_.FoundCRLF()) {
            extraction_ = parser_.ExtractLine();
            return EXTRACTION_SUCCESS;
        }
        if (parser_.FoundSingleCR()) {
            LOG(INFO) << "Single CR found in Line";
            return EXTRACTION_FOUND_SINGLE_CR;
        }
        if (parser_.Peek() == '\0') {
            LOG(INFO) << "Null Terminator found in Line";
            return EXTRACTION_NULL_TERMINATOR_FOUND;
        }
        parser_.Advance();
    }
    return EXTRACTION_CRLF_NOT_FOUND;
}

RequestBuilder::ExtractionResult RequestBuilder::TryToExtractBodyContent_()
{
    while (!parser_.EndOfBuffer()) {
        if (parser_.ElementLen() > body_builder_.max_body_size) {
            LOG(INFO) << "Body content too long";
            return EXTRACTION_TOO_LONG;
        }
        if (parser_.FoundCRLF()) {
            extraction_ = parser_.ExtractLine();
            return EXTRACTION_SUCCESS;
        }
        parser_.Advance();
    }
    return EXTRACTION_CRLF_NOT_FOUND;
}

bool RequestBuilder::IsParsingState_(BuildState state) const
{
    return (state != BS_MATCH_SERVER && state != BS_CHECK_HEADERS &&
            state != BS_PREPARE_TO_READ_BODY);
}

ResponseCode RequestBuilder::InsertHeaderField_(std::string& key, std::string& value)
{
    header_count_++;
    if (header_count_ > RQ_MAX_HEADER_COUNT) {
        return HTTP_REQUEST_HEADER_FIELDS_TOO_LARGE;
    }
    std::string key_lower = utils::ToLowerCase(key);
    // todo: handle list values
    rq_.headers[key_lower] = value;
    return HTTP_OK;
}

RequestBuilder::BuildState RequestBuilder::SetStatusAndExitBuilder_(ResponseCode status)
{
    rq_.status = status;
    return BS_BAD_REQUEST;
}

}  // namespace http
