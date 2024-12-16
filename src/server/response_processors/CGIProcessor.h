#ifndef WS_SERVER_RESPONSE_PROCESSORS_CGI_PROCESSOR_H
#define WS_SERVER_RESPONSE_PROCESSORS_CGI_PROCESSOR_H

#include <Location.h>
#include <SocketWrapper.h>
#include <multiplexers/ICallback.h>
#include <shared_ptr.h>
#include <unique_ptr.h>

#include "AResponseProcessor.h"

class CGIProcessor : public AResponseProcessor {
  private:
    int Execute_(const std::string& script_path, const std::string& interpreter,
                 const http::Request& rq);
    bool IsValidMethod_(utils::shared_ptr<Location> loc, const http::Request& rq);

    class ReadChildOutputCallback : public c_api::ICallback {
      public:
        ReadChildOutputCallback(CGIProcessor& processor);
        virtual void Call(int);

      private:
        CGIProcessor& processor_;
    };

    class ChildProcess {
      public:
        ChildProcess(pid_t pid);
        void Monitor();
        int status() const;

      private:
        pid_t child_pid_;
        std::time_t start_time_;
        int status_;
    };

  public:
    CGIProcessor(const Server& server, const std::string& script_path, const http::Request& rq,
                 utils::shared_ptr<Location> loc,
                 utils::unique_ptr<http::IResponseCallback> response_rdy_cb);
    ~CGIProcessor();

  private:
    utils::unique_ptr<c_api::SocketWrapper> wrapped_socket_;
    std::vector<char> buffer_;
};

#endif  // WS_SERVER_RESPONSE_PROCESSORS_CGI_PROCESSOR_H

// Requirements from subject:

// ◦ Execute CGI based on certain file extension (for example .php).
// ◦ Make it work with POST and GET methods.
// ◦ Make the route able to accept uploaded files and configure where they should
// be saved.
// ∗ Because you won’t call the CGI directly, use the full path as PATH_INFO.
// ∗ Just remember that, for chunked request, your server needs to unchunk
// it, the CGI will expect EOF as end of the body.
// ∗ Same things for the output of the CGI. If no content_length is returned
// from the CGI, EOF will mark the end of the returned data.
// ∗ Your program should call the CGI with the file requested as first argument.
// ∗ The CGI should be run in the correct directory for relative path file access.
// ∗ Your server should work with one CGI (php-CGI, Python, and so forth).
