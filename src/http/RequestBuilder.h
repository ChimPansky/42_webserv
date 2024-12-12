#ifndef WS_HTTP_REQUEST_BUILDER_H
#define WS_HTTP_REQUEST_BUILDER_H

#include "Request.h"
#include "RequestParser.h"
#include "ResponseCodes.h"
#include <unique_ptr.h>

#include <cstddef>
#include <cstdio>
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

struct ChosenServerParams {
    int max_body_size;
};

class IChooseServerCb {
  public:
    virtual ChosenServerParams Call(const http::Request& rq) = 0;
    virtual ~IChooseServerCb() {};
};


class RequestBuilder {
  private:
    struct BodyBuilder {
        BodyBuilder();

        std::ofstream body_stream;
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
    RequestBuilder(utils::unique_ptr<IChooseServerCb> choose_server_cb = utils::unique_ptr<IChooseServerCb>(NULL));
    void PrepareToRecvData(size_t recv_size);
    void AdjustBufferSize(size_t bytes_recvd);
    void Build(size_t bytes_recvd);
    RqBuilderStatus builder_status() const;
    const Request& rq() const;
    std::vector<char>& buf();

  private:
    Request rq_;
    RqBuilderStatus builder_status_;
    RequestParser parser_;
    std::string extraction_;
    BuildState build_state_;
    std::string header_key_;
    BodyBuilder body_builder_;
    utils::unique_ptr<IChooseServerCb> choose_server_cb_;

    BuildState BuildFirstLine_();
    http::ResponseCode TrySetMethod_(const std::string& raw_method);
    http::ResponseCode TrySetRqTarget_(const std::string& raw_rq_target);
    http::ResponseCode TrySetVersion_(const std::string& raw_version);
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
    BuildState MatchServer_();
    BuildState BuildBodyRegular_();
    BuildState BuildBodyChunkSize_();
    BuildState BuildBodyChunkContent_();

    // helpers:
    bool CanBuild_();
    void NullTerminatorCheck_(char c);
    ExtractionResult TryToExtractLine_();
    ExtractionResult TryToExtractBodyContent_();
    bool IsParsingState_(BuildState state) const;
    BuildState SetStatusAndExitBuilder_(ResponseCode status);
};

}  // namespace http

#endif  // WS_HTTP_REQUEST_BUILDER_H
