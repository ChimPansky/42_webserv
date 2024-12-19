#ifndef WS_SERVER_RESPONSE_PROCESSORS_A_RESPONSE_PROCESSOR_H
#define WS_SERVER_RESPONSE_PROCESSORS_A_RESPONSE_PROCESSOR_H

#include <Request.h>
#include <Response.h>
#include <logger.h>
#include <time_utils.h>
#include <unique_ptr.h>

class Server;

class AResponseProcessor {
  protected:
    AResponseProcessor(const Server& server,
                       utils::unique_ptr<http::IResponseCallback> response_rdy_cb)
        : server_(server), response_rdy_cb_(response_rdy_cb){};

    void DelegateToErrProc(http::ResponseCode err_code);

  public:
    virtual ~AResponseProcessor(){};

  protected:
    const Server& server_;
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
