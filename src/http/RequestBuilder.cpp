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
#include "file_utils.h"
#include "http.h"
#include "str_utils.h"
#include "unique_ptr.h"

namespace http {

RequestBuilder::RequestBuilder(utils::unique_ptr<IOnHeadersReadyCb> choose_server_cb)
    : builder_status_(RB_BUILDING),
      build_state_(BS_RQ_LINE),
      headers_ready_cb_(choose_server_cb),
      header_count_(0),
      header_section_size_(0),
      rq_has_body_(false)
{
    if (!headers_ready_cb_) {
        throw std::logic_error("No Choose Server Callback specified");
    }
}

RequestBuilder::BodyBuilder::BodyBuilder()
    : chunked(false), body_idx(0), remaining_length(0), max_body_size(0)
{}

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

void RequestBuilder::Build(const char* data, size_t data_size)
{
    if (parser_.EndOfBuffer() && data_size == 0) {
        // TODO why if throw here test dont pass? data_size == 0 should never happen
        LOG(INFO) << "Ran out of data while Request incomplete.";
        rq_.status = HTTP_BAD_REQUEST;
        builder_status_ = RB_DONE;
        return;
    }
    parser_.AddNewData(data, data_size);
    while (CanBuild_()) {
        switch (build_state_) {
            case BS_RQ_LINE: build_state_ = BuildFirstLine_(); break;
            case BS_HEADER_FIELDS: build_state_ = BuildHeaderField_(); break;
            case BS_AFTER_HEADERS: build_state_ = ProcessHeaders_(); break;
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
    switch (TryToExtractLine_()) {
        case EXTRACTION_SUCCESS: break;
        case EXTRACTION_CRLF_NOT_FOUND: return BS_RQ_LINE;
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
    utils::maybe<Method> converted_method = HttpMethodFromStr(raw_method);
    if (!converted_method) {
        LOG(INFO) << "Method not implemented: " << raw_method;
        return HTTP_NOT_IMPLEMENTED;
    }
    rq_.method = *converted_method;
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
    utils::maybe<Version> converted_version = HttpVersionFromStr(raw_version);
    if (!converted_version) {
        LOG(INFO) << "Version not implemented: " << raw_version;
        return HTTP_BAD_REQUEST;
    }
    rq_.version = *converted_version;
    return HTTP_OK;
}

RequestBuilder::BuildState RequestBuilder::BuildHeaderField_()
{
    // LOG(DEBUG) << "BuildHeaderField_";
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
        return BS_AFTER_HEADERS;
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
    // LOG(INFO) << "Could not insert header Field";
    if (rc != http::HTTP_OK) {
        LOG(INFO) << "Could not insert header Field";
        return SetStatusAndExitBuilder_(rc);
    }
    return BS_HEADER_FIELDS;
}

RequestBuilder::BuildState RequestBuilder::ProcessHeaders_()
{
    ResponseCode rc = ValidateHeadersSyntax_();
    if (rc != http::HTTP_OK) {
        return SetStatusAndExitBuilder_(rc);
    }
    rc = InterpretHeaders_();
    if (rc != http::HTTP_OK) {
        return SetStatusAndExitBuilder_(rc);
    }
    LOG(DEBUG) << "Request: " << rq_.GetDebugString();
    HeadersValidationResult validation_result = headers_ready_cb_->Call(rq_);
    if (validation_result.status != HTTP_OK) {
        LOG(DEBUG) << "ChooseServerCb returned error: " << validation_result.status;
        return SetStatusAndExitBuilder_(validation_result.status);
    }
    if (!rq_has_body_) {
        LOG(DEBUG) << "Request has no body";
        return BS_END;
    }
    LOG(DEBUG) << "Request has body, max_body_size: " << validation_result.max_body_size;
    body_builder_.max_body_size = validation_result.max_body_size;
    if (validation_result.upload_path.ok()) {
        const std::string& upload_path = *validation_result.upload_path;
        if (utils::DoesPathExist(upload_path.c_str())) {
            return SetStatusAndExitBuilder_(HTTP_CONFLICT);
        }
        body_builder_.body_stream.open(upload_path.c_str());
        if (!body_builder_.body_stream.is_open()) {
            LOG(ERROR) << "Failed to open upload location: " << upload_path;
            return SetStatusAndExitBuilder_(HTTP_INTERNAL_SERVER_ERROR);
        }
        rq_.body.path = upload_path;
        rq_.body.storage_type = BST_ON_SERVER;
    } else {
        LOG(DEBUG) << "No upload path specified, using temporary file";
        utils::maybe<std::string> tmp_f =
            utils::CreateAndOpenTmpFileToStream(body_builder_.body_stream);
        LOG(DEBUG) << "Created temporary file: " << *tmp_f;
        if (!tmp_f) {
            LOG(ERROR) << "Failed to create temporary file.";
            return SetStatusAndExitBuilder_(HTTP_INTERNAL_SERVER_ERROR);
        }
        rq_.body.path = *tmp_f;
        rq_.body.storage_type = BST_IN_TMP_FOLDER;
    }
    return BS_PREPARE_TO_READ_BODY;
}

ResponseCode RequestBuilder::ValidateHeadersSyntax_()
{
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
    return HTTP_OK;
}

// todo: do we need to differentiate btw HTTP/1.0 and HTTP/1.1?
ResponseCode RequestBuilder::InterpretHeaders_()
{
    utils::maybe<std::string> host = rq_.GetHeaderVal("host");
    if (!host.ok()) {
        LOG(INFO) << "Host header missing";
        return HTTP_BAD_REQUEST;
    }
    utils::maybe<std::string> content_length = rq_.GetHeaderVal("content-length");
    utils::maybe<std::string> transfer_encoding = rq_.GetHeaderVal("transfer-encoding");
    if (content_length && transfer_encoding) {
        LOG(INFO) << "Both Content-Length and Transfer-Encoding headers present";
        return HTTP_BAD_REQUEST;
    }
    if (transfer_encoding && *transfer_encoding == "chunked") {
        rq_has_body_ = true;
        body_builder_.chunked = true;
    }
    if (content_length) {
        rq_has_body_ = true;
    }
    if (rq_.method == HTTP_POST && !content_length && !transfer_encoding) {
        LOG(INFO) << "POST request without Content-Length or Transfer-Encoding";
        return HTTP_LENGTH_REQUIRED;
    }
    // additional semantic checks...
    return HTTP_OK;
}


RequestBuilder::BuildState RequestBuilder::PrepareBody_()
{
    LOG(DEBUG) << "PrepareBody_";
    if (body_builder_.chunked) {
        return BS_BODY_CHUNK_SIZE;
    }
    utils::maybe<std::string> content_length = rq_.GetHeaderVal("content-length");
    if (!content_length) {
        LOG(INFO) << "Content-Length missing";
        return SetStatusAndExitBuilder_(HTTP_LENGTH_REQUIRED);
    }
    utils::maybe<size_t> content_length_num = utils::StrToNumericNoThrow<size_t>(*content_length);
    if (!content_length_num) {
        LOG(INFO) << "Content-Length not a number";
        return SetStatusAndExitBuilder_(HTTP_BAD_REQUEST);
    }
    if (*content_length_num > body_builder_.max_body_size) {
        LOG(INFO) << "Content-Length too large";
        return SetStatusAndExitBuilder_(HTTP_PAYLOAD_TOO_LARGE);
    }
    body_builder_.remaining_length = *content_length_num;
    return BS_BODY_REGULAR;
}

RequestBuilder::BuildState RequestBuilder::BuildBodyRegular_()
{
    LOG(DEBUG) << "BuildBodyRegular_";
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
    rq_.body.transfer_complete = true;
    return BS_END;
}

RequestBuilder::BuildState RequestBuilder::BuildBodyChunkSize_()
{
    LOG(DEBUG) << "BuildBodyChunkSize_";
    switch (TryToExtractLine_()) {
        case EXTRACTION_SUCCESS: break;
        case EXTRACTION_CRLF_NOT_FOUND: return BS_BODY_CHUNK_SIZE;
        default: return SetStatusAndExitBuilder_(HTTP_BAD_REQUEST);
    }
    utils::maybe<size_t> chunk_size = utils::HexToUnsignedNumericNoThrow<size_t>(extraction_);
    if (!chunk_size) {
        LOG(INFO) << "Chunk Size Syntax Error";
        return SetStatusAndExitBuilder_(HTTP_BAD_REQUEST);
    }
    if (*chunk_size == 0) {
        body_builder_.body_stream.close();
        rq_.body.transfer_complete = true;
        return BS_END;
    }
    body_builder_.body_idx += *chunk_size;
    body_builder_.remaining_length = *chunk_size;
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
    return (state != BS_AFTER_HEADERS && state != BS_PREPARE_TO_READ_BODY);
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
