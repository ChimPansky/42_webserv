#ifndef WS_HTTP_REQUEST_BUILDER_H
#define WS_HTTP_REQUEST_BUILDER_H

#include "Request.h"
#include "RequestParser.h"
#include "ResponseCodes.h"

#include <cstddef>
#include <string>
#include <vector>

namespace http {

enum RqBuilderStatus {
    RB_BUILDING,
    RB_NEED_DATA_FROM_CLIENT,
    RB_NEED_TO_MATCH_SERVER,
    RB_BUILD_BODY_REGULAR,
    RB_BUILD_BODY_CHUNKED,
    RB_DONE
};

class RequestBuilder {
  private:
    struct BodyBuilder {
        BodyBuilder(std::vector<char>* rq_body);

        void ExpandBuffer(size_t additional_size);
        std::vector<char>* body;
        bool chunked;
        size_t body_idx;
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
    RequestBuilder();
    void PrepareToRecvData(size_t recv_size);
    void AdjustBufferSize(size_t bytes_recvd);
    void Build(size_t bytes_recvd);
    void ApplyServerInfo(size_t max_body_size);
    RqBuilderStatus builder_status() const;
    const Request& rq() const;
    std::vector<char>& buf();

  private:
    Request rq_;
    RqBuilderStatus builder_status_;
    RequestParser parser_;
    std::string extraction_;
    ExtractionResult extraction_result_;
    BuildState build_state_;
    std::string header_key_;
    BodyBuilder body_builder_;

    BuildState BuildFirstLine_();
    http::ResponseCode ValidateFirstLine_(std::string& raw_method, std::string& raw_rq_target, std::string& raw_version);
    BuildState BuildHeaderField_();
    http::ResponseCode ValidateHeadersSyntax_();
    http::ResponseCode InterpretHeaders_();

    bool InsertHeaderField_(std::string& key, std::string& value);

    BuildState BuildMethod_();
    BuildState BuildRqTarget_();
    BuildState BuildVersion_();
    BuildState CheckForNextHeader_();
    BuildState BuildHeaderKey_();
    BuildState ParseHeaderKeyValSep_();
    BuildState BuildHeaderValue_();
    BuildState ReadyForServer_();
    BuildState BuildBodyRegular_();
    BuildState BuildBodyChunkSize_();
    BuildState BuildBodyChunkContent_();

    // helpers:
    bool CanBuild_();
    void NullTerminatorCheck_(char c);
    ExtractionResult TryToExtractLine_();
    ExtractionResult TryToExtractBodyContent_();
    bool IsParsingState_(BuildState state) const;
    BuildState Error_(ResponseCode status);
};

}  // namespace http

#endif  // WS_HTTP_REQUEST_BUILDER_H
