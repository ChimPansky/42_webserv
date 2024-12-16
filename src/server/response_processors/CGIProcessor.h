#ifndef WS_SERVER_RESPONSE_PROCESSORS_CGI_PROCESSOR_H
#define WS_SERVER_RESPONSE_PROCESSORS_CGI_PROCESSOR_H

#include <SocketWrapper.h>
#include <fcntl.h>
#include <multiplexers/ICallback.h>
#include <shared_ptr.h>
#include <sys/wait.h>
#include <unique_ptr.h>
#include <unistd.h>

#include <fstream>

#include "AResponseProcessor.h"
#include "Location.h"

class CGIProcessor : public AResponseProcessor {
  private:
    std::string GetInterpreterByExt_(utils::shared_ptr<Location> loc, const std::string& script);
    void SetEnv_(const std::string& script_path, const http::Request& rq);
    int Execute_(const std::string& script_path, const http::Request& rq);
    bool IsValidMethod_(utils::shared_ptr<Location> loc, const http::Request& rq);

    class CGIReadCallback : public c_api::ICallback {
      public:
        CGIReadCallback(c_api::SocketWrapper& socket, std::vector<char>& buffer);
        virtual void Call(int);

      private:
        c_api::SocketWrapper& socket_;
        std::vector<char>& buffer_;
    };

    class CGIWriteCallback : public c_api::ICallback {
      public:
        CGIWriteCallback(c_api::SocketWrapper& socket, std::vector<char> content);
        virtual void Call(int);

      private:
        c_api::SocketWrapper& socket_;
        std::vector<char> buf_;
        size_t buf_send_idx_;
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
    CGIProcessor(const std::string& script_path, const http::Request& rq,
                 utils::shared_ptr<Location> loc,
                 utils::unique_ptr<http::IResponseCallback> response_rdy_cb);
    ~CGIProcessor();

  private:
    utils::unique_ptr<c_api::SocketWrapper> wrapped_socket_;
    std::string interpreter_;
    std::vector<char> buffer_;
    std::vector<std::string> env_;
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
