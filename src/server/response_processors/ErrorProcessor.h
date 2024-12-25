#ifndef WS_SERVER_RESPONSE_PROCESSORS_ERROR_RESPONSE_PROCESSOR_H
#define WS_SERVER_RESPONSE_PROCESSORS_ERROR_RESPONSE_PROCESSOR_H

#include <Response.h>

#include "AResponseProcessor.h"

class ErrorProcessor : public AResponseProcessor {
  private:
    class GeneratedErrorProcessor : public AResponseProcessor {
      public:
        GeneratedErrorProcessor(RequestDestination dest,
                                utils::unique_ptr<http::IResponseCallback> response_rdy_cb,
                                http::ResponseCode code);

      private:
        std::string GenerateErrorPage_(http::ResponseCode code);
    };

  public:
    ErrorProcessor(RequestDestination dest,
                   utils::unique_ptr<http::IResponseCallback> response_rdy_cb,
                   http::ResponseCode code);
};

#endif  // WS_SERVER_RESPONSE_PROCESSORS_ERROR_RESPONSE_PROCESSOR_H
