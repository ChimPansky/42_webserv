#ifndef WS_HTTP_REQUEST_BUILDER_H
#define WS_HTTP_REQUEST_BUILDER_H

#include <cstddef>
#include <vector>

#include "Request.h"

namespace http {

class RequestBuilder {
    class   BodyBuilder {
      public:
        BodyBuilder(std::vector<char> *rq_body);

        std::vector<char> *body;
        bool    chunked;
        size_t  chunk_size;
        size_t  idx;
        size_t  remaining_length;
        size_t  max_body_size;

        bool Complete() const;
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
        BS_BODY_REGULAR_CHECK_LENGTH,
        BS_BODY_REGULAR,
        BS_BODY_CHUNK_SIZE,
        BS_BODY_CHUNK_CONTENT,
        BS_END,
        BS_BAD_REQUEST
    };

  public:
    RequestBuilder();
    bool HasReachedEndOfBuffer() const;
    size_t ProcessBuffer(size_t bytes_read);
    bool IsReadyForResponse();
    bool needs_info_from_server() const;
    void set_max_body_size(size_t max_body_size);
    const Request& rq() const;
    std::vector<char>& buf();

  private:
    Request rq_;
    size_t crlf_counter_;
    std::vector<char> buf_;
    size_t begin_idx_;
    size_t end_idx_;
    BuildState build_state_;
    bool found_space_;
    std::string header_key_;
    bool needs_info_from_server_;
    BodyBuilder body_builder_;


    size_t ParseLen_() const;
    char GetNextChar_(void);
    void NullTerminatorCheck_(char c);
    int CompareBuf_(const char*, size_t len) const;
    void UpdateBeginIdx_(void);
    bool ReadingBody_(void) const;

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

    bool DoesBodyExceedMaxSize_() const;

    void PrintParseBuf_() const;
};

}  // namespace http

#endif  // WS_HTTP_REQUEST_BUILDER_H
