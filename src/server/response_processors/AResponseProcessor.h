#ifndef WS_SERVER_RESPONSE_PROCESSORS_A_RESPONSE_PROCESSOR_H
#define WS_SERVER_RESPONSE_PROCESSORS_A_RESPONSE_PROCESSOR_H

#include <Request.h>
#include <Response.h>
#include <logger.h>
#include <time_utils.h>
#include <unique_ptr.h>

#include "../Server.h"

class AResponseProcessor {
  protected:
    AResponseProcessor(RequestDestination dest,
                       utils::unique_ptr<http::IResponseCallback> response_rdy_cb)
        : dest_(dest), response_rdy_cb_(response_rdy_cb) {};

    void DelegateToErrProc(http::ResponseCode err_code);

  public:
    virtual ~AResponseProcessor() {};

  protected:
    RequestDestination dest_;
    utils::unique_ptr<http::IResponseCallback> response_rdy_cb_;
    utils::unique_ptr<AResponseProcessor> delegated_processor_;
};

// class HelloWorldResponseProcessor : public AResponseProcessor {
//   public:
//     HelloWorldResponseProcessor(const Server& server, utils::unique_ptr<http::IResponseCallback>
//     response_rdy_cb)
//         : AResponseProcessor(server, response_rdy_cb)
//     {
//         response_rdy_cb_->Call(http::GetSimpleValidResponse());
//     }
// };

#endif  // WS_SERVER_ARESPONSE_PROCESSOR_H
