#ifndef WS_SERVER_RESPONSE_PROCESSORS_CGI_PROCESSOR_H
#define WS_SERVER_RESPONSE_PROCESSORS_CGI_PROCESSOR_H

#include <ChildProcessesManager.h>
#include <Location.h>
#include <SocketWrapper.h>
#include <multiplexers/ICallback.h>
#include <shared_ptr.h>
#include <unique_ptr.h>

#include "AResponseProcessor.h"

class CGIProcessor : public AResponseProcessor {
  public:
    CGIProcessor(const Server& server, const std::string& script_path, const http::Request& rq,
                 const std::vector<std::string>& allowed_cgi_extensions,
                 utils::unique_ptr<http::IResponseCallback> response_rdy_cb);
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
    class ChildProcessDoneCb : public c_api::IChildDiedCb {
      public:
        ChildProcessDoneCb(CGIProcessor& processor) : processor_(processor) {}
        virtual void Call(int child_exit_status);

      private:
        CGIProcessor& processor_;
    };

  private:
    utils::unique_ptr<c_api::SocketWrapper> parent_socket_;
    std::vector<char> cgi_out_buffer_;
};

#endif  // WS_SERVER_RESPONSE_PROCESSORS_CGI_PROCESSOR_H
