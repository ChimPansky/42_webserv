#ifndef WS_HTTP_REQUEST_BUILDER_H
#define WS_HTTP_REQUEST_BUILDER_H

#include <unique_ptr.h>

#include <fstream>
#include <string>
#include <vector>

#include "Request.h"
#include "RequestParser.h"
#include "ResponseCodes.h"


namespace http {

enum RqBuilderStatus {
    RB_BUILDING,
    RB_NEED_DATA_FROM_CLIENT,
    RB_DONE
};

struct HeadersValidationResult {
    HeadersValidationResult(ResponseCode status) : status(status) {};
    int max_body_size;
    utils::maybe<std::string> upload_path;
    ResponseCode status;
};

class IOnHeadersReadyCb {
  public:
    virtual HeadersValidationResult Call(const http::Request& rq) = 0;
    virtual ~IOnHeadersReadyCb() {};
};


class RequestBuilder {
  private:
    struct BodyBuilder {
        BodyBuilder();

        void Reset();

        std::ofstream body_stream;
        bool chunked;
        size_t body_len;
        size_t remaining_length;
        size_t max_body_size;
    };

  private:
    enum BuildState {
        BS_RQ_LINE,
        BS_HEADER_FIELDS,
        BS_AFTER_HEADERS,
        BS_BODY_REGULAR,
        BS_BODY_CHUNK_SIZE,
        BS_BODY_CHUNK_CONTENT,
        BS_END,
        BS_BAD_REQUEST
    };
    enum ExtractionResult {
        EXTRACTION_SUCCESS,
        EXTRACTION_TOO_LONG,
        EXTRACTION_CRLF_NOT_FOUND,
        EXTRACTION_FOUND_SINGLE_CR,
        EXTRACTION_NULL_TERMINATOR_FOUND
    };

  public:
    RequestBuilder(utils::unique_ptr<IOnHeadersReadyCb> choose_server_cb);
    void Build(const char* data, size_t data_sz);
    RqBuilderStatus builder_status() const;
    const Request& rq() const;
    std::vector<char>& buf();
    void Reset();


  private:
    Request rq_;
    RqBuilderStatus builder_status_;
    RequestParser parser_;
    std::string extraction_;
    BuildState build_state_;
    BodyBuilder body_builder_;
    utils::unique_ptr<IOnHeadersReadyCb> headers_ready_cb_;
    size_t header_count_;
    size_t header_section_size_;
    bool rq_has_body_;

    BuildState BuildFirstLine_();
    http::ResponseCode TrySetMethod_(const std::string& raw_method);
    http::ResponseCode TrySetRqTarget_(const std::string& raw_rq_target);
    http::ResponseCode TrySetVersion_(const std::string& raw_version);
    BuildState BuildHeaderField_();
    BuildState ProcessHeaders_();
    http::ResponseCode ValidateHeadersSyntax_();
    http::ResponseCode InterpretHeaders_();
    ResponseCode InsertHeaderField_(std::string& key, std::string& value);
    RequestBuilder::BuildState PrepareBody_(const HeadersValidationResult& validation_result);
    BuildState BuildBodyRegular_();
    BuildState BuildBodyChunkSize_();
    BuildState BuildBodyChunkContent_();

    // helpers:
    bool CanBuild_();
    void NullTerminatorCheck_(char c);
    ExtractionResult TryToExtractLine_();
    ExtractionResult TryToExtractBodyContent_();
    BuildState SetStatusAndExitBuilder_(ResponseCode status);
};

}  // namespace http

#endif  // WS_HTTP_REQUEST_BUILDER_H
