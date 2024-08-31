#ifndef WS_HTTP_REQUEST_BUILDER_H
#define WS_HTTP_REQUEST_BUILDER_H

#include "Request.h"
namespace http {

class RequestBuilder {
  private:
    enum ParseState {
        PS_START,
        PS_METHOD,
        PS_URI,
        PS_VERSION,
        PS_HEADERS,
        PS_BODY,
        PS_END,
        PS_ERROR
    };

  public:
    RequestBuilder();
    
  private:
    http::Request rq_;

};

}  // namespace http

#endif  // WS_HTTP_REQUEST_BUILDER_H
