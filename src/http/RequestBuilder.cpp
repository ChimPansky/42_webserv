#include "RequestBuilder.h"
#include "Request.h"
#include "ResponseCodes.h"

#include <cstring>
#include <string>
#include <vector>

#include <logger.h>
#include <numeric_utils.h>
#include "SyntaxChecker.h"
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
    // utils::Logger::get().set_severity_threshold(INFO);
    LOG(DEBUG) << "RequestBuilder::Build";
    // client session will be killed earlier, so dead code, rm
    if (parser_.EndOfBuffer() && bytes_recvd == 0) {
        rq_.status = HTTP_BAD_REQUEST;
        builder_status_ = RB_DONE;
        return;
    }
    while (CanBuild_()) {
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
        case EXTRACTION_SUCCESS: break;
        case EXTRACTION_CRLF_NOT_FOUND: return BS_RQ_LINE;
        default: return Error_(HTTP_BAD_REQUEST);
    }
    // todo for robustness: if very first line of request empty -> ignore and continue
    std::stringstream ss(line_);
    std::string raw_method, raw_rq_target, raw_version;
    std::getline(ss, raw_method, ' ');
    if (ss.eof()) {
        return Error_(HTTP_BAD_REQUEST);
    }
    std::getline(ss, raw_rq_target, ' ');
    if (ss.eof()) {
        return Error_(HTTP_BAD_REQUEST);
    }
    std::getline(ss, raw_version);
    if (!ss.eof()) {
        return Error_(HTTP_BAD_REQUEST);
    }
    ResponseCode rc = ValidateFirstLine_(raw_method, raw_rq_target, raw_version);
    if (rc != http::HTTP_OK) {
        return Error_(rc);
    }
    return BS_HEADER_FIELDS;
}

ResponseCode RequestBuilder::ValidateFirstLine_(std::string& raw_method, std::string& raw_rq_target, std::string& raw_version) {
    LOG(INFO) << "ValidateFirstLine_";
    LOG(INFO) << "raw_method: {" << raw_method << "}";
    LOG(INFO) << "raw_rq_target: " << raw_rq_target;
    LOG(INFO) << "raw_version: {" << raw_version << "}";
    if (!SyntaxChecker::IsValidMethod(raw_method)) {
        return HTTP_BAD_REQUEST;
    };
    std::pair<bool, Method> converted_method = StrToHttpMethod(raw_method);
    if (!converted_method.first) {
        return HTTP_NOT_IMPLEMENTED;
    }
    rq_.method = converted_method.second;
    rq_.rqTarget = raw_rq_target;
    if (rq_.rqTarget.validity_state() & RqTarget::RQ_TARGET_TOO_LONG) {
        return HTTP_URI_TOO_LONG;
    }
    if (!rq_.rqTarget.Good()) {
        return HTTP_BAD_REQUEST;
    }
    if (!SyntaxChecker::IsValidVersion(raw_version)) {
        return HTTP_BAD_REQUEST;
    };
    std::pair<bool, Version> converted_version = StrToHttpVersion(raw_version);
    if (!converted_version.first) {
        return HTTP_HTTP_VERSION_NOT_SUPPORTED;
    }
    rq_.version = converted_version.second;
    LOG(INFO) << "converted_version: " << rq_.version;
    return HTTP_OK;
}

// https://www.rfc-editor.org/rfc/rfc9110#name-field-lines-and-combined-fi
// When a field name is only present once in a section, the combined "field value" for that field consists of the corresponding field line value. When a field name is repeated within a section, its combined field value consists of the list of corresponding field line values within that section, concatenated in order, with each field line value separated by a comma.

// For example, this section:

// Example-Field: Foo, Bar
// Example-Field: Baz
// contains two field lines, both with the field name "Example-Field". The first field line has a field line value of "Foo, Bar", while the second field line value is "Baz". The field value for "Example-Field" is the list "Foo, Bar, Baz".
RequestBuilder::BuildState RequestBuilder::BuildHeaderField_() {
    // todo: store last header_key and if it's the same as the current one, append to the value
    LOG(INFO) << "BuildHeaderField_";
    extraction_result_ = TryToExtractLine_();
    switch (extraction_result_) {
        case EXTRACTION_SUCCESS: break;
        case EXTRACTION_CRLF_NOT_FOUND: return BS_HEADER_FIELDS;
        default: return Error_(HTTP_BAD_REQUEST);
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
    ss >> header_val;
    ss >> std::ws;
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
    for (std::map<std::string, std::string>::const_iterator it = rq_.headers.begin();
         it != rq_.headers.end(); ++it) {
        if (!SyntaxChecker::IsValidHeaderKey(it->first)) {
            return HTTP_BAD_REQUEST;
        }
        if (!SyntaxChecker::IsValidHeaderValue(it->second)) {
            return HTTP_BAD_REQUEST;
        }
    }
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
        case EXTRACTION_SUCCESS: break;
        case EXTRACTION_CRLF_NOT_FOUND: return BS_BODY_CHUNK_SIZE;
        default: return Error_(HTTP_BAD_REQUEST);
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
        case EXTRACTION_SUCCESS: break;
        case EXTRACTION_TOO_LONG: return Error_(HTTP_BAD_REQUEST);
        default: return BS_BODY_CHUNK_CONTENT;
    }
    size_t old_sz = body_builder_.body->size();
    body_builder_.body->resize(old_sz + body_builder_.remaining_length);
    std::memcpy(body_builder_.body->data() + old_sz, line_.data(), line_.size());
    return BS_BODY_CHUNK_SIZE;
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
        if (parser_.FoundSingleCR()) {
            return EXTRACTION_FOUND_SINGLE_CR;
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
