#include "RequestBuilder.h"
#include "Request.h"
#include "ResponseCodes.h"

#include <cctype>
#include <cstring>
#include <string>
#include <vector>

#include <logger.h>
#include <numeric_utils.h>
#include "str_utils.h"

namespace http {

RequestBuilder::RequestBuilder()
    : builder_status_(RB_BUILDING), build_state_(BS_RQ_LINE), body_builder_(&rq_.body)
{}

RequestBuilder::BodyBuilder::BodyBuilder(std::vector<char>* rq_body)
    : body(rq_body), chunked(false), body_idx(0), remaining_length(0), max_body_size(0)
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
    LOG(INFO) << "buffer: " << parser_.buf().data();
    // client session will be killed earlier, so dead code, rm
    if (parser_.EndOfBuffer() && bytes_recvd == 0) {
        rq_.status = HTTP_BAD_REQUEST;
        builder_status_ = RB_DONE;
        return;
    }
    while (CanBuild_()) {
        if (IsParsingState_(build_state_)) {       // todo: check for \0 within states...
            NullTerminatorCheck_(parser_.Peek());  // can there be \0 in body???
        }
        switch (build_state_) {
            case BS_RQ_LINE:            build_state_ = BuildFirstLine_(); break;
            case BS_HEADER_FIELDS:       build_state_ = BuildHeaderField_(); break;
            case BS_AFTER_HEADERS:      build_state_ = NeedToMatchServer_(); break;
            case BS_CHECK_FOR_BODY:     build_state_ = CheckForBody_(); break;
            case BS_CHECK_BODY_REGULAR_LENGTH:  build_state_ = CheckBodyRegularLength_(); break;
            case BS_BODY_REGULAR:               build_state_ = BuildBodyRegular_(); break;
            case BS_BODY_CHUNK_SIZE:            build_state_ = BuildBodyChunkSize_(); break;
            case BS_BODY_CHUNK_CONTENT:         build_state_ = BuildBodyChunkContent_(); break;
            case BS_BAD_REQUEST: break;
            case BS_END: break;
        }
        if (builder_status_ == RB_NEED_TO_MATCH_SERVER) {
            LOG(DEBUG) << "NEED_TO_MATCH_SERVER -> break out of Build-Loop";
            return;
        }
    }
    if (build_state_ == BS_END || build_state_ == BS_BAD_REQUEST) {
        builder_status_ = RB_DONE;
    }
}

void RequestBuilder::ApplyServerInfo(size_t max_body_size)
{
    body_builder_.max_body_size = max_body_size;
    if (parser_.EndOfBuffer()) {
        builder_status_ = RB_NEED_DATA_FROM_CLIENT;
    } else {
        builder_status_ = RB_BUILDING;
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
    LOG(INFO) << "BuildFirstLine_";
    extraction_result_ = TryToExtractLine_();
    switch (extraction_result_) {
        case EXTRACTION_CRLF_NOT_FOUND: return BS_RQ_LINE;
        case EXTRACTION_NULL_TERMINATOR_FOUND: return Error_(HTTP_BAD_REQUEST);
        case EXTRACTION_TOO_LONG: return Error_(HTTP_BAD_REQUEST);
        case EXTRACTION_SUCCESS: break;
    }
    // todo for robustness: if very first line of request empty -> ignore and continue
    std::stringstream ss(line_);
    std::getline(ss, raw_method_, ' ');
    if (ss.eof()) {
        return Error_(HTTP_BAD_REQUEST);
    }
    std::getline(ss, raw_uri_, ' ');
    if (ss.eof()) {
        return Error_(HTTP_BAD_REQUEST);
    }
    std::getline(ss, raw_version_);
    if (!ss.eof()) {
        return Error_(HTTP_BAD_REQUEST);
    }
    ResponseCode rc = ValidateFirstLine_();
    if (rc != http::HTTP_OK) {
        return Error_(rc);
    }
    return BS_HEADER_FIELDS;

}

ResponseCode RequestBuilder::ValidateFirstLine_() {
    LOG(INFO) << "ValidateFirstLine_";
    // if (!syntaxchecker.check_method(raw_method)) {
    //     return HTTP_BAD_REQUEST;
    // };
    if (raw_method_ == "GET") {
        rq_.method = HTTP_GET;
    } else if (raw_method_ == "POST") {
        rq_.method = HTTP_POST;
    } else if (raw_method_ == "DELETE") {
        rq_.method = HTTP_DELETE;
    } else {
        return HTTP_NOT_IMPLEMENTED;
    }
    rq_.uri = raw_uri_; // todo: change this to Uri object once merged
    // if Uri bad...
    // if (!syntaxchecker.check_version(raw_version)) {
    //     return HTTP_BAD_REQUEST;
    // };
    if (raw_version_ == "HTTP/1.0") {
        rq_.version = HTTP_1_0;
    } else if (raw_version_ == "HTTP/1.1") {
        rq_.version = HTTP_1_1;
    } else {
        return HTTP_HTTP_VERSION_NOT_SUPPORTED;
    }
    return HTTP_OK;
}

RequestBuilder::BuildState RequestBuilder::BuildHeaderField_() {
    LOG(INFO) << "BuildHeaderField_";
    extraction_result_ = TryToExtractLine_();
    switch (extraction_result_) {
        case EXTRACTION_CRLF_NOT_FOUND: return BS_HEADER_FIELDS;
        case EXTRACTION_NULL_TERMINATOR_FOUND: return Error_(HTTP_BAD_REQUEST);
        case EXTRACTION_TOO_LONG: return Error_(HTTP_BAD_REQUEST);
        case EXTRACTION_SUCCESS: break;
    }
    if (line_.empty()) {
        ResponseCode rc = ValidateHeaders_();
        if (rc != http::HTTP_OK) {
            return Error_(rc);
        }
        return BS_AFTER_HEADERS;
    }
    std::stringstream ss(line_);
    std::string header_key, header_val;
    std::getline(ss, header_key, ':');
    if (ss.eof()) {
        return Error_(HTTP_BAD_REQUEST);
    }
    std::getline(ss, header_val);
    if (!ss.eof()) {
        return Error_(HTTP_BAD_REQUEST);
    }
    if (!InsertHeaderField_(header_key, header_val)) {
        return Error_(HTTP_BAD_REQUEST);
    }
    return BS_HEADER_FIELDS;
}

ResponseCode RequestBuilder::ValidateHeaders_()
{
    LOG(INFO) << "ValidateHeaders_";
    // iterate through headers map
    // for each header: if syntaxcheck bad --> return HTTP_BAD_REQUEST
    // if duplicate host header --> return HTTP_BAD_REQUEST
    // additional semantic checks...
    return HTTP_OK;
}

RequestBuilder::BuildState RequestBuilder::NeedToMatchServer_() {
    builder_status_ = http::RB_NEED_TO_MATCH_SERVER;
    return BS_CHECK_FOR_BODY;
}

RequestBuilder::BuildState RequestBuilder::CheckForBody_()
{
    if (rq_.method == HTTP_GET || rq_.method == HTTP_DELETE) {
        return BS_END;
    }
    std::pair<bool, std::string> content_length = rq_.GetHeaderVal("content-length");
    std::pair<bool, std::string> transfer_encoding = rq_.GetHeaderVal("transfer-encoding");
    if (content_length.first && transfer_encoding.first) {
        return Error_(HTTP_BAD_REQUEST);
    }
    if (transfer_encoding.second == "chunked") {
        body_builder_.chunked = true;
        return BS_BODY_CHUNK_SIZE;
    }
    if (content_length.first) {
        std::pair<bool, size_t> content_length_num =
            utils::StrToNumericNoThrow<size_t>(content_length.second);
        if (content_length_num.first) {
            body_builder_.remaining_length =
                content_length_num.second;  // TODO: content-length limits?
            return BS_CHECK_BODY_REGULAR_LENGTH;
        } else {
            return Error_(HTTP_BAD_REQUEST);
        }
    }
    if (rq_.method == HTTP_POST) {
        return Error_(HTTP_BAD_REQUEST);
    }
    return BS_END;
}

RequestBuilder::BuildState RequestBuilder::CheckBodyRegularLength_()
{
    if (body_builder_.remaining_length > body_builder_.max_body_size) {
        return Error_(HTTP_BAD_REQUEST);
    }
    body_builder_.body->resize(body_builder_.remaining_length);
    return BS_BODY_REGULAR;
}

RequestBuilder::BuildState RequestBuilder::BuildBodyRegular_()
{
    while (!parser_.EndOfBuffer() && body_builder_.remaining_length > 0) {
        parser_.Advance();
    }
    size_t copy_size = std::min(parser_.ElementLen(), body_builder_.remaining_length);
    std::memcpy(body_builder_.body->data() + body_builder_.body_idx,
                parser_.buf().data(), copy_size);
                // todo erase from buf
    body_builder_.body_idx += copy_size;
    body_builder_.remaining_length -= copy_size;
    if (body_builder_.remaining_length == 0) {
        return BS_END;
    }
    parser_.Advance();
    return BS_BODY_REGULAR;
}

// https://datatracker.ietf.org/doc/html/rfc2616#section-3.5
RequestBuilder::BuildState RequestBuilder::BuildBodyChunkSize_()
{
    LOG(INFO) << "BuildBodyChunkSize_";
    extraction_result_ = TryToExtractLine_();
    switch (extraction_result_) {
        case EXTRACTION_CRLF_NOT_FOUND: return BS_BODY_CHUNK_SIZE;
        case EXTRACTION_NULL_TERMINATOR_FOUND: return Error_(HTTP_BAD_REQUEST);
        case EXTRACTION_TOO_LONG: return Error_(HTTP_BAD_REQUEST);
        case EXTRACTION_SUCCESS: break;
    }
    std::pair<bool, size_t> converted_size =
        utils::HexToUnsignedNumericNoThrow<size_t>(line_);
    if (!converted_size.first) {
        return Error_(HTTP_BAD_REQUEST);
    }
    body_builder_.remaining_length =
        converted_size.second;  // TODO: check for chunk size limits (or is it client_max_body_size?)
    if (rq_.body.size() + body_builder_.remaining_length > body_builder_.max_body_size) {
        return Error_(HTTP_BAD_REQUEST);
    }
    if (body_builder_.remaining_length == 0) {
        return BS_END;
    }
    return BS_BODY_CHUNK_CONTENT;
}

RequestBuilder::BuildState RequestBuilder::BuildBodyChunkContent_()
{
    LOG(INFO) << "BuildBodyChunkContent_";
    extraction_result_ = TryToExtractBodyContent_();
    switch (extraction_result_) {
        case EXTRACTION_CRLF_NOT_FOUND: return BS_BODY_CHUNK_CONTENT;
        case EXTRACTION_TOO_LONG: return Error_(HTTP_BAD_REQUEST);
        case EXTRACTION_SUCCESS: break;
        default: break;
    }
    size_t old_sz = body_builder_.body->size();
    body_builder_.body->resize(old_sz + body_builder_.remaining_length);
    std::memcpy(body_builder_.body->data() + old_sz, line_.data(), line_.size());
    return BS_BODY_CHUNK_SIZE;
}

void RequestBuilder::NullTerminatorCheck_(char c)
{
    if (c == '\0' && build_state_ != BS_BODY_CHUNK_CONTENT) {
        build_state_ = BS_BAD_REQUEST;
    }
}

RequestBuilder::ExtractionResult RequestBuilder::TryToExtractLine_() {
    while (!parser_.EndOfBuffer()) {
        if (parser_.ElementLen() > RQ_LINE_LEN_LIMIT) {
            return EXTRACTION_TOO_LONG;
        }
        if (parser_.FoundCRLF()) {
            line_ = parser_.ExtractLine();
            return EXTRACTION_SUCCESS;
        }
        if (parser_.Peek() == '\0') {
            return EXTRACTION_NULL_TERMINATOR_FOUND;
        }
        parser_.Advance();
    }
    return EXTRACTION_CRLF_NOT_FOUND;
}

RequestBuilder::ExtractionResult RequestBuilder::TryToExtractBodyContent_() {
    while (!parser_.EndOfBuffer()) {
        if (parser_.ElementLen() > body_builder_.max_body_size) {
            return EXTRACTION_TOO_LONG;
        }
        if (parser_.FoundCRLF()) {
            line_ = parser_.ExtractLine();
            return EXTRACTION_SUCCESS;
        }
        parser_.Advance();
    }
    return EXTRACTION_CRLF_NOT_FOUND;
}

bool RequestBuilder::IsParsingState_(BuildState state) const
{
    return (state != BS_AFTER_HEADERS && state != BS_CHECK_FOR_BODY && state != BS_CHECK_BODY_REGULAR_LENGTH);
}

bool RequestBuilder::InsertHeaderField_(std::string& key, std::string& value) {
    std::string key_lower = utils::ToLowerCase(key);
    // todo: check for host-duplicates
    // todo: handle list values
    rq_.headers[key_lower] = value;
    return true;
}

RequestBuilder::BuildState RequestBuilder::Error_(ResponseCode status) {
    rq_.status = status;
    return BS_BAD_REQUEST;
}

}  // namespace http
