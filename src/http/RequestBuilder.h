#ifndef WS_HTTP_REQUEST_BUILDER_H
#define WS_HTTP_REQUEST_BUILDER_H

#include <cstddef>
#include <vector>


#include "Request.h"
#include "RequestParser.h"

class Server;
namespace http {

enum RqBuilderStatus {
    RB_BUILDING,
    RB_NEED_DATA_FROM_CLIENT,
    RB_NEED_INFO_FROM_SERVER,
    RB_DONE
};

class RequestBuilder {
  private:
    struct BodyBuilder {
        BodyBuilder(std::vector<char> *rq_body);

        std::vector<char> *body;
        bool    chunked;
        size_t  body_idx;
        size_t  remaining_length;
        size_t  max_body_size;
    };

  private:
    enum BuildState {
        BS_METHOD,
        BS_URI,
        BS_VERSION,
        BS_BETWEEN_HEADERS,
        BS_HEADER_KEY,
        BS_HEADER_KEY_VAL_SEP,
        BS_HEADER_VALUE,
        BS_CHECK_FOR_BODY,
        BS_CHECK_BODY_REGULAR_LENGTH,
        BS_BODY_REGULAR,
        BS_BODY_CHUNK_SIZE,
        BS_BODY_CHUNK_CONTENT,
        BS_END,
        BS_BAD_REQUEST
    };
    enum EOL_CHARS {
        EOL_CARRIAGE_RETURN = '\r',
        EOL_LINE_FEED = '\n'
    };

  public:
    RequestBuilder();
    void PrepareToRecvData(size_t recv_size);
    void Build(size_t bytes_recvd);
    void ApplyServerInfo(size_t max_body_size);
    RqBuilderStatus builder_status() const;
    const Request& rq() const;
    std::vector<char>& buf();

  private:
    Request rq_;
    RqBuilderStatus builder_status_;
    RequestParser parser_;
    BuildState build_state_;
    std::string header_key_;
    BodyBuilder body_builder_;

    BuildState BuildMethod_();
    BuildState BuildUri_();
    BuildState BuildVersion_();
    BuildState CheckForNextHeader_();
    BuildState BuildHeaderKey_();
    BuildState ParseHeaderKeyValSep_();
    BuildState BuildHeaderValue_();
    BuildState CheckForBody_();
    BuildState CheckBodyRegularLength_();
    BuildState BuildBodyRegular_();
    BuildState BuildBodyChunkSize_();
    BuildState BuildBodyChunkContent_();

    // helpers:
    void AdjustBufferSize_(size_t bytes_recvd);
    bool CanBuild_();
    void NullTerminatorCheck_(char c);
    bool CheckForEOL_() const;
    bool IsParsingState_(BuildState state) const;
};

}  // namespace http

#endif  // WS_HTTP_REQUEST_BUILDER_H
