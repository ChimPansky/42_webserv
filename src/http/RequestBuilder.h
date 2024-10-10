#ifndef WS_HTTP_REQUEST_BUILDER_H
#define WS_HTTP_REQUEST_BUILDER_H

#include <cstddef>
#include <vector>

#include "Request.h"

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
    struct   BodyBuilder {
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
    void Build(size_t bytes_read);
    void ApplyServerInfo(size_t max_body_size);
    RqBuilderStatus builder_status() const;
    const Request& rq() const;
    std::vector<char>& buf();

  private:
    Request rq_;
    RqBuilderStatus builder_status_;
    std::vector<char> buf_;
    size_t begin_idx_;
    size_t end_idx_;
    BuildState build_state_;
    std::string header_key_;
    BodyBuilder body_builder_;

    BuildState BuildMethod_(void);
    BuildState BuildUri_(char c);
    BuildState BuildVersion_(void);
    BuildState CheckForNextHeader_(char c);
    BuildState BuildHeaderKey_(char c);
    BuildState ParseHeaderKeyValSep_(char c);
    BuildState BuildHeaderValue_(char c);
    BuildState CheckForBody_(void);
    BuildState CheckBodyRegularLength_(void);
    BuildState BuildBodyRegular_(void);
    BuildState BuildBodyChunkSize_(char c);
    BuildState BuildBodyChunkContent_(void);

    // helpers:
    bool CanBuild_(void);
    size_t ParseLen_() const;
    int CompareBuf_(const char*, size_t len) const;
    void NullTerminatorCheck_(char c);
    void UpdateBeginIdx_(void);
    bool CheckForEOL_() const;
    bool HasReachedEndOfBuffer_(void) const;
    bool IsBodyReadingState_(BuildState state) const;
    bool IsProcessingState_(BuildState state) const;
};

}  // namespace http

#endif  // WS_HTTP_REQUEST_BUILDER_H
