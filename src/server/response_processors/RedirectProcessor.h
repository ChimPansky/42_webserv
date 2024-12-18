#ifndef WS_SERVER_RESPONSE_PROCESSORS_REDIRECT_PROCESSOR_H
#define WS_SERVER_RESPONSE_PROCESSORS_REDIRECT_PROCESSOR_H

#include <Response.h>

#include "AResponseProcessor.h"

class RedirectProcessor : public AResponseProcessor {
  public:
    RedirectProcessor(const Server& server,
                                utils::unique_ptr<http::IResponseCallback> response_rdy_cb,
                                http::ResponseCode code, const std::string& redirect_uri);
};

#endif  // WS_SERVER_RESPONSE_PROCESSORS_REDIRECT_PROCESSOR_H
