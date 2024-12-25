#ifndef WS_SERVER_RESPONSE_PROCESSORS_CGI_PROCESSOR_H
#define WS_SERVER_RESPONSE_PROCESSORS_CGI_PROCESSOR_H

#include <ChildProcessesManager.h>
#include <Location.h>
#include <Socket.h>
#include <cgi/cgi.h>
#include <multiplexers/ICallback.h>
#include <shared_ptr.h>
#include <unique_ptr.h>

#include "AResponseProcessor.h"

class CGIProcessor : public AResponseProcessor {
  public:
    CGIProcessor(RequestDestination dest,
                 utils::unique_ptr<http::IResponseCallback> response_rdy_cb,
                 const http::Request& rq);
    ~CGIProcessor();

  private:
    class ReadChildOutputCallback : public c_api::ICallback {
      public:
        ReadChildOutputCallback(CGIProcessor& processor) : processor_(processor) {}
        virtual void Call(int);

      private:
        CGIProcessor& processor_;
    };

  private:
    // TODO: client session closed before cb is invoked
    // dont include proc mb, make it autonomus
    class ChildProcessDoneCb : public c_api::IChildDiedCb {
      public:
        ChildProcessDoneCb(CGIProcessor& processor) : processor_(processor) {}
        virtual void Call(int child_exit_status);

      private:
        CGIProcessor& processor_;
    };

  private:
    utils::maybe<c_api::ChildProcessDescription> child_process_description_;
    std::vector<char> cgi_out_buffer_;
    bool ready_to_rs_;
    bool rs_sent_;
};

#endif  // WS_SERVER_RESPONSE_PROCESSORS_CGI_PROCESSOR_H
