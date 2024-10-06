#include "RequestBuilder.h"

#include <cctype>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

#include "Request.h"
#include "utils/logger.h"
#include "utils/utils.h"
#include "utils.h"

namespace http {

RequestBuilder::RequestBuilder()
    : chunk_counter_(0), crlf_counter_(0), begin_idx_(0), end_idx_(0), build_state_(BS_METHOD), found_space_(false), needs_info_from_server_(false)
{}
const Request& RequestBuilder::rq() const
{
    return rq_;
}

bool RequestBuilder::needs_info_from_server() const
{
    return needs_info_from_server_;
}

void RequestBuilder::set_max_body_size(size_t max_body_size)
{
    rq_.body.max_body_size = max_body_size;
}

std::vector<char>& RequestBuilder::buf()
{
    return buf_;
}

bool RequestBuilder::HasReachedEndOfBuffer() const
{
    return end_idx_ == buf_.size();
}

size_t RequestBuilder::ParseNext(size_t bytes_read)
{
    // LOG(DEBUG) << "RequestBuilder::ParseNext: bytes_read: " << bytes_read;
    ++chunk_counter_;
    size_t iterations = 0;
    ////LOG(DEBUG) << "Parsing chunk no " << chunk_counter_;
    ////LOG(DEBUG) << "buf_.size(): " << buf_.size() << "; end_idx_: " << end_idx_;
    if (bytes_read == 0 && ReadingBody_() && !rq_.body.Complete() && !needs_info_from_server_) {
        //LOG(DEBUG) << "EOF reached while reading body and body is not complete -> bad_request";
        build_state_ = BS_BAD_REQUEST;
    }
    while (!IsReadyForResponse() && (!HasReachedEndOfBuffer() || build_state_ == BS_CHECK_FOR_BODY) && build_state_ != BS_BAD_REQUEST) {
        iterations++;
        //LOG(DEBUG) << "before getchar: ParseLen: " << ParseLen_() << "; begin_idx: " << begin_idx_ << "; end_idx: " << end_idx_ << "; buf.size(): " << buf_.size();
        char c = GetNextChar_();
        //LOG(DEBUG) << "after  getchar; ParseLen: " << ParseLen_() << "; begin_idx: " << begin_idx_ << "; end_idx: " << end_idx_ << "; buf.size(): " << buf_.size() << "; c: " << (int)c;
        bool state_changed = false;
        NullTerminatorCheck_(c);
        switch (build_state_) {
            case BS_METHOD:
                build_state_ = BuildMethod_();
                state_changed = (build_state_ != BS_METHOD ? true : false);
                break;
            case BS_URI:
                build_state_ = BuildUri_(c);
                state_changed = (build_state_ != BS_URI ? true : false);
                break;
            case BS_VERSION:
                build_state_ = BuildVersion_();
                state_changed = (build_state_ != BS_VERSION ? true : false);
                break;
            case BS_BETWEEN_HEADERS:
                build_state_ = CheckForNextHeader_(c);
                state_changed = (build_state_ != BS_BETWEEN_HEADERS ? true : false);
                break;
            case BS_HEADER_KEY:
                build_state_ = BuildHeaderKey_(c);
                state_changed = (build_state_ != BS_HEADER_KEY ? true : false);
                break;
            case BS_HEADER_KEY_VAL_SEP:
                build_state_ = ParseHeaderKeyValSep_(c);
                state_changed = (build_state_ != BS_HEADER_KEY_VAL_SEP ? true : false);
                break;
            case BS_HEADER_VALUE:
                build_state_ = BuildHeaderValue_(c);
                state_changed = (build_state_ != BS_HEADER_VALUE ? true : false);
                break;
            case BS_CHECK_FOR_BODY:
                build_state_ = CheckForBody_();
                state_changed = (build_state_ != BS_CHECK_FOR_BODY ? true : false);
                if (ReadingBody_()) {
                    needs_info_from_server_ = true;
                    return iterations;
                }
                break;
            case BS_BODY_REGULAR:
                needs_info_from_server_ = false;
                build_state_ = BuildBodyRegular_();
                state_changed = (build_state_ != BS_BODY_REGULAR ? true : false);
                break;
            case BS_BODY_CHUNK_SIZE:
                needs_info_from_server_ = false;
                build_state_ = BuildBodyChunkSize_(c);
                state_changed = (build_state_ != BS_BODY_CHUNK_SIZE ? true : false);
                break;
            case BS_BODY_CHUNK_CONTENT:
                build_state_ = BuildBodyChunkContent_();
                state_changed = (build_state_ != BS_BODY_CHUNK_CONTENT ? true : false);
                break;
            case BS_END:
                break;
            case BS_BAD_REQUEST:
                break;
        }
        if (state_changed) {
            UpdateBeginIdx_();
        }
    }
    if (build_state_ == BS_BAD_REQUEST || iterations == 0) {
        //LOG(DEBUG) << "After loop: BS_BAD_REQUEST -> bad_request!";
        rq_.bad_request = true;
    }
    if (build_state_ == BS_END) {
        //LOG(DEBUG) << "After loop: BS_END -> rq_complete!";
        rq_.rq_complete = true;
    }
    return iterations;
}

bool RequestBuilder::IsReadyForResponse()
{
    //LOG(DEBUG) << "RequestBuilder::IsReadyForResponse: EOF: " << eof_checker_.end_of_file_
    //           << "Bad Request: " << rq_.bad_request_;
    return (rq_.rq_complete || build_state_ == BS_BAD_REQUEST /*body_complete()*/);
}
size_t RequestBuilder::ParseLen_() const
{
    return end_idx_ - begin_idx_;
}

char RequestBuilder::GetNextChar_()
{
    if (build_state_ == BS_CHECK_FOR_BODY) {
        return ' ';
    }
    return buf_[end_idx_++];
}

void RequestBuilder::NullTerminatorCheck_(char c)
{
    if (c == '\0' && build_state_ != BS_CHECK_FOR_BODY && build_state_ != BS_BODY_CHUNK_CONTENT) {
        //LOG(DEBUG) << "Null terminator found and not currently reading chunked body content -> bad_request";
        build_state_ = BS_BAD_REQUEST;
    }
}

int RequestBuilder::CompareBuf_(const char* str, size_t len) const
{
    return std::strncmp(buf_.data() + begin_idx_, str, len);
}

void RequestBuilder::UpdateBeginIdx_()
{
    ////LOG(DEBUG) << "RequestBuilder::UpdateBeginIdx_";
    begin_idx_ = end_idx_;
}

bool RequestBuilder::ReadingBody_() const
{
    return (build_state_ == BS_BODY_REGULAR || build_state_ == BS_BODY_CHUNK_SIZE || build_state_ == BS_BODY_CHUNK_CONTENT);
}

RequestBuilder::BuildState RequestBuilder::BuildMethod_()
{
    if (ParseLen_() == 4 && CompareBuf_("GET ", 4) == 0) {
        rq_.method = HTTP_GET;
    }
    if (ParseLen_() == 5 && CompareBuf_("POST ", 5) == 0) {
        rq_.method = HTTP_POST;
    }
    if (ParseLen_() == 7 && CompareBuf_("DELETE ", 7) == 0) {
        rq_.method = HTTP_DELETE;
    }
    if (rq_.method != HTTP_NO_METHOD) {
        return BS_URI;
    }
    if (ParseLen_() > 7) {
        return BS_BAD_REQUEST;
    }
    return BS_METHOD;
}

//https://datatracker.ietf.org/doc/html/rfc2616#page-17
// https://datatracker.ietf.org/doc/html/rfc2396
RequestBuilder::BuildState RequestBuilder::BuildUri_(char c)
{
    ////LOG(DEBUG) << "Parsing URI... char: " << c;
    if (c == ' ') {
        if (ParseLen_() > 1) {
            rq_.uri = std::string(buf_.data() + begin_idx_, buf_.data() + end_idx_ - 1);
            return BS_VERSION;
        } else {
            return BS_BAD_REQUEST;
        }
    }
    return BS_URI;
}

RequestBuilder::BuildState RequestBuilder::BuildVersion_(void)
{
    //LOG(DEBUG) << "Parsing Version. ParseLen: " << ParseLen_() << "; buf: " << std::string(buf_.data() + begin_idx_, ParseLen_());
    if (ParseLen_() == 10 && CompareBuf_("HTTP/0.9\r\n", 10) == 0) {
        rq_.version = HTTP_0_9;
    }
    if (ParseLen_() == 10 && CompareBuf_("HTTP/1.0\r\n", 10) == 0) {
        rq_.version = HTTP_1_0;
    }
    if (ParseLen_() == 10 && CompareBuf_("HTTP/1.1\r\n", 10) == 0) {
        rq_.version = HTTP_1_1;
    }
    if (ParseLen_() == 8 && CompareBuf_("HTTP/2\r\n", 8) == 0) {
        rq_.version = HTTP_2;
    }
    if (ParseLen_() == 8 && CompareBuf_("HTTP/3\r\n", 8) == 0) {
        rq_.version = HTTP_3;
    }
    if (rq_.version != HTTP_NO_VERSION) {
        return BS_BETWEEN_HEADERS;
    }
    if (ParseLen_() > 10) {
        return BS_BAD_REQUEST;
    }
    return BS_VERSION;
}

RequestBuilder::BuildState RequestBuilder::CheckForNextHeader_(char c)
{
    ////LOG(DEBUG) << "Checking for next header... chr:  (" << (int)c << ")";
    if (ParseLen_() == 1) {
        if (c == '\r') {
            //LOG(DEBUG) << "Found carriage return..";
            crlf_counter_++;
            return BS_BETWEEN_HEADERS;
        }
        //LOG(DEBUG) << "Found random char -> end_idx_-- and go to BS_HEADER_KEY...";
        end_idx_--;
        return BS_HEADER_KEY;
    }
    if (ParseLen_() == 2) {
        if (c == '\n') {
            //LOG(DEBUG) << "Found empty line -> End of Headers";
            crlf_counter_ = 0;
            return BS_CHECK_FOR_BODY;
        }
        crlf_counter_ = 0;
        //LOG(DEBUG) << "Found random char after carriage return -> bad_request...";
        return BS_BAD_REQUEST;
    }
    return BS_BAD_REQUEST; // should never reach this point...
}

RequestBuilder::BuildState RequestBuilder::BuildHeaderKey_(char c)
{
    ////LOG(DEBUG) << "Parsing Header-Key... char: " << c;
    if (c == ':') {
        if (ParseLen_() == 1) {
            LOG(ERROR) << "Request-Header key is invalid: " << std::string(buf_.data() + begin_idx_, ParseLen_() - 1);
            return BS_BAD_REQUEST;
        } else {
            // ExtractSubstrLowerCase_(buf_, header_key_, begin_idx_, ParseLen_() - 1);
            header_key_ = std::string(buf_.data() + begin_idx_, ParseLen_() - 1);
            found_space_ = false;
            // TODO: check if header_key already exists in map (no duplicates allowed)
            return BS_HEADER_KEY_VAL_SEP;
        }
    } else if (!(std::isalnum(c) || (ParseLen_() > 1 && c == '-'))) { // TODO: check this condition...
        //LOG(DEBUG) << "Request-Header key can only contain alphanumeric chars and '-' -> bad_request";
        return BS_BAD_REQUEST;
    }
    buf_[end_idx_ - 1] = std::tolower(c);
    return BS_HEADER_KEY;
}

RequestBuilder::BuildState RequestBuilder::ParseHeaderKeyValSep_(char c)
{
    //LOG(DEBUG) << "Parsing Header-Separator..." ;
    if (std::isspace(c)) {
        found_space_ = true;
        return BS_HEADER_KEY_VAL_SEP;
    }
    if (!found_space_) {
        //LOG(DEBUG) << "No space found after ':' -> bad_request";
        return BS_BAD_REQUEST;
    }
    end_idx_--;
    return BS_HEADER_VALUE;
}
// RFC: https://datatracker.ietf.org/doc/html/rfc2616#page-17
RequestBuilder::BuildState RequestBuilder::BuildHeaderValue_(char c)
{
    //LOG(DEBUG) << "Parsing Header-Value... ";
    if (crlf_counter_ == 0) {
        if (c == '\r') {
            if (ParseLen_() <= 1) {
                return BS_BAD_REQUEST;
            }
            crlf_counter_++;
            return BS_HEADER_VALUE;
        }
        if (!std::isprint(c)) { // TODO: additional checks for valid characters...
            //LOG(DEBUG) << "Request-Header value is invalid -> bad_request";
            return BS_BAD_REQUEST;
        }
    }
    if (crlf_counter_ == 1) {
        if (c == '\n') {
            crlf_counter_ = 0;
            //LOG(DEBUG) << "HeaderValue complete -> inserting into map...";
            // ExtractSubstrLowerCase_(buf_, rq_.headers[header_key_], begin_idx_, ParseLen_() - 2);
            rq_.headers[header_key_] = std::string(buf_.data() + begin_idx_, ParseLen_() - 2);
            return BS_BETWEEN_HEADERS;
        }
        crlf_counter_ = 0;
        //LOG(DEBUG) << "carriage return without line feed found in header-value -> bad_request";
        return BS_BAD_REQUEST;

    }
    return BS_HEADER_VALUE;
}

RequestBuilder::BuildState RequestBuilder::CheckForBody_(void)
{
    //LOG(DEBUG) << "After Headers, checking if body is expected...";
    if (rq_.method == HTTP_GET || rq_.method == HTTP_DELETE) {
        // LOG(DEBUG) << "No Body expected for GET/DELETE -> request complete";
        return BS_END;
    }
    std::string content_length = rq_.GetHeaderVal("content-length");
    std::string transfer_encoding = rq_.GetHeaderVal("transfer-encoding");
    if (!content_length.empty() && !transfer_encoding.empty()) {
        //LOG(DEBUG) << "Both Content-Length and Transfer-Encoding found -> Bad Request";
        return BS_BAD_REQUEST;
    }
    if (transfer_encoding == "chunked") {
        //LOG(DEBUG) << "Chunked Transfer-Encoding found -> Go read body as chunks";
        rq_.body.chunked = true;
        return BS_BODY_CHUNK_SIZE;
    }
    if (!content_length.empty()) {
        std::pair<bool, size_t> content_length_num = utils::StrToNumericNoThrow<size_t>(content_length);
        //LOG(DEBUG) << "Content-Length found: " << content_length_num.second;
        if (content_length_num.first) {
            //LOG(DEBUG) << "Content-lenght is numeric";
            rq_.body.remaining_length = content_length_num.second; // TODO: content-length limits?
            rq_.body.content.resize(content_length_num.second);
        } else {
            //LOG(DEBUG) << "Content-lenght is non-numeric -> Bad Request";
            return BS_BAD_REQUEST;
        }
        if (rq_.body.remaining_length != 0) {
            //LOG(DEBUG) << "Content-Length > 0 -> Go read body according to Content-Length";
            return BS_BODY_REGULAR;
        }
    }
    if (rq_.method == HTTP_POST) {
        //LOG(DEBUG) << "Method is POST, but no Content-Length or Transfer-Encoding found -> Bad Request";
        return BS_BAD_REQUEST;
    }
    //LOG(DEBUG) << "No Content-Length or Transfer-Encoding found -> No Body expected --> request complete...";
    return BS_END;
}

RequestBuilder::BuildState RequestBuilder::BuildBodyRegular_(void)
{
    if (HasReachedEndOfBuffer()) {
        size_t copy_size = std::min(buf_.size() - begin_idx_, rq_.body.remaining_length);
        std::memcpy(rq_.body.content.data() + rq_.body.content_idx, buf_.data() + begin_idx_, copy_size);
        rq_.body.content_idx += copy_size;
        rq_.body.remaining_length -= copy_size;
        UpdateBeginIdx_();
        //LOG(DEBUG) << "Copied " << copy_size << " bytes to body; body_idx: " << rq_.body.content_idx << "; remaining: " << rq_.body.remaining_length;
        if (DoesBodyExceedMaxSize_()) {
            //LOG(DEBUG) << "Body size exceeds max_body_size -> Bad Request...";
            return BS_BAD_REQUEST;
        }
        if (rq_.body.remaining_length == 0) {
            //LOG(DEBUG) << "Body complete: " << rq_.body.content.data();
            //LOG(DEBUG) << "Body size: " << rq_.body.content.size();
            return BS_END;
        }
    }
    return BS_BODY_REGULAR;
}

// https://datatracker.ietf.org/doc/html/rfc2616#section-3.5
RequestBuilder::BuildState RequestBuilder::BuildBodyChunkSize_(char c)
{
    //LOG(DEBUG) << "Reading Body ChunkSize...";
    buf_[end_idx_ - 1] = std::tolower(c);
    if (crlf_counter_ == 0) {
        if (c == '\r') {
            //LOG(DEBUG) << "Found carriage return..";
            crlf_counter_++;
        }
        return BS_BODY_CHUNK_SIZE;
    }
    if (crlf_counter_ == 1) {
        if (c == '\n') {
            //LOG(DEBUG) << "Found line feed. Converting hex to size_t: " << std::string(buf_.data() + begin_idx_, ParseLen_() - 2);
            crlf_counter_ = 0;
            std::pair<bool, size_t> converted_size = HexStrToSizeT(std::string(buf_.data() + begin_idx_, ParseLen_() - 2));
            if (!converted_size.first) {
                //LOG(DEBUG) << "Read invalid ChunkSize -> Bad Request...";
                return BS_BAD_REQUEST;
            }
            rq_.body.chunk_size = converted_size.second; // TODO: check for chunk size limits
            //LOG(DEBUG) << "Read ChunkSize: " << rq_.body.chunk_size;
            if (rq_.body.chunk_size == 0) {
                //LOG(DEBUG) << "Read ChunkSize 0 -> Request complete!";
                return BS_END;
            }
            return BS_BODY_CHUNK_CONTENT;
        }
        crlf_counter_ = 0;
        return BS_BAD_REQUEST;
    }
    return BS_BODY_CHUNK_SIZE;
}

RequestBuilder::BuildState RequestBuilder::BuildBodyChunkContent_(void)
{
    //LOG(DEBUG) << "Reading Body ChunkContent...";
    if (ParseLen_() + rq_.body.content.size() > rq_.body.max_body_size) {
        //LOG(DEBUG) << "Body size exceeds max_body_size -> Bad Request...";
        return BS_BAD_REQUEST;
    }
    if (ParseLen_() == rq_.body.chunk_size) {
        size_t old_sz = rq_.body.content.size();
        rq_.body.content.resize(old_sz + rq_.body.chunk_size);
        std::memcpy(rq_.body.content.data() + old_sz, buf_.data() + begin_idx_, rq_.body.chunk_size);
        //LOG(DEBUG) << "Copied " << rq_.body.chunk_size << " bytes to body; body_size: " << rq_.body.content.size();
        if (DoesBodyExceedMaxSize_()) {
            //LOG(DEBUG) << "Body size exceeds max_body_size -> Bad Request...";
            return BS_BAD_REQUEST;
        }
        return BS_BODY_CHUNK_CONTENT;
    }
    if (ParseLen_() == rq_.body.chunk_size + 1) {
        if (buf_[end_idx_ - 1] != '\r') {
            //LOG(DEBUG) << "Expected CR after chunk content -> Bad Request...";
            return BS_BAD_REQUEST;
        }
        return BS_BODY_CHUNK_CONTENT;
    }
    if (ParseLen_() == rq_.body.chunk_size + 2) {
        if (buf_[end_idx_ - 1] != '\n') {
            //LOG(DEBUG) << "Expected LF after CR after chunk content -> Bad Request...";
            return BS_BAD_REQUEST;
        }
        return BS_BODY_CHUNK_SIZE;
    }
    // return BS_BODY_CHUNK_CONTENT;
    return BS_BODY_CHUNK_CONTENT;
}

bool RequestBuilder::DoesBodyExceedMaxSize_() const {
    //LOG(DEBUG) << "BODYSIZE? size: " << rq_.body.content.size() << "; max: " << rq_.body.max_body_size;
    return rq_.body.content.size() > rq_.body.max_body_size;
}

void RequestBuilder::PrintParseBuf_() const
{
    std::cout << "PrintParseBuf(): Len: " << buf_.size() << "; Content: |";
    for (size_t i = 0; i < buf_.size(); i++) {
        std::cout << buf_[i];
    }
    std::cout << "|" << std::endl;
}

}  // namespace http


// int main() {
//     std::string hex_str1 = "10000000";
//     std::string hex_str2 = "FFFFFFFFFFFFFFFF";
//     std::pair<bool, size_t> result1 = http::HexStrToSizeT_(hex_str1);
//     std::pair<bool, size_t> result2 = http::HexStrToSizeT_(hex_str2);
//     std::cout << "Result: " << result1.first << "; " << result1.second << std::endl;
//     std::cout << "Result: " << result2.first << "; " << result2.second << std::endl;
// }
