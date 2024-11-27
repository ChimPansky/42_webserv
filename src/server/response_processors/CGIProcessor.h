#ifndef WS_SERVER_RESPONSE_PROCESSORS_CGI_PROCESSOR_H
#define WS_SERVER_RESPONSE_PROCESSORS_CGI_PROCESSOR_H

#include <unique_ptr.h>

#include <fstream>

#include "AResponseProcessor.h"

class CGIProcessor : public AResponseProcessor {
  private:
    void    SetEnv_(const http::Request& rq)
    {
        std::vector<std::string> envv;

        envv.push_back("GATEWAY_INTERFACE=CGI/1.1");
        envv.push_back("SERVER_PROTOCOL=HTTP/1.1");
        envv.push_back("SCRIPT_NAME=" + cgi_path_);
        if (rq.method == http::HTTP_POST) {
            envv.push_back("REQUEST_METHOD=POST");
            envv.push_back("CONTENT_LENGTH=" + rq.body.size());
            envv.push_back("CONTENT_TYPE=" + rq.GetHeaderVal("Content-Type").second);
        } else if (rq.method == http::HTTP_GET) {
            envv.push_back("REQUEST_METHOD=GET");
            envv.push_back("QUERY_STRING=" + rq.url.query());
        } else if (rq.method == http::HTTP_DELETE) {
            envv.push_back("REQUEST_METHOD=DELETE");
        }
        envv.push_back("HTTP_HOST=" +
                    rq.GetHeaderVal("Host").second);  // required for all HTTP/1.1 requests
        envv.push_back("PATH_INFO=" + rq.url());
        envv.push_back("SERVER_NAME=" + rq.GetHeaderVal("Host").second);  // rq.url.host();
        // envv.push_back("HTTP_COOKIE=" + rq.GetHeaderVal("Cookie").second); bonuses

        envv.push_back("SERVER_SOFTWARE=webserv/1.0");
        envv.push_back("SERVER_PORT=...");
        envv.push_back("REMOTE_PORT=...");
        envv.push_back("REMOTE_ADDR=...");
        envv.push_back("PATH_TRANSLATED=");  // root + uri
        envv.push_back(
            "HTTP_USER_AGENT=" +
            rq.GetHeaderVal("User-Agent").second);  // Browser the client is using to send the request.
                                                    // General format: software/version library/version.
        envv.push_back("HTTP_ACCEPT=" +
                    rq.GetHeaderVal("Accept")
                        .second);  // This can be useful if your CGI script handles multiple types of
                                    // responses but is not strictly necessary.

        for (size_t i = 0; i < envv.size(); ++i) {
            envv_.push_back(const_cast<char*>(envv[i].c_str()));
        }
        envv_.push_back(NULL);
    }

    int Execute(int clientsocket)
    {
        pid_t child_pid = fork();
        if (child_pid < 0) {
            std::perror("fork failed");
            return -1;
        } else if (child_pid == 0) {
            dup2(clientsocket, STDOUT_FILENO);
            dup2(clientsocket, STDERR_FILENO);

            char* args[] = {const_cast<char*>(cgi_path_.c_str()), NULL};
            execve(cgi_path_.c_str(), args, &envv_[0]);
            std::perror("execve failed");
            exit(1);
        }
        waitpid(child_pid, NULL, WNOHANG);
        return 0;
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
    }

  public:
    CGIProcessor(const std::string& file_path,
                  utils::unique_ptr<http::IResponseCallback> response_rdy_cb) : err_response_processor(utils::unique_ptr<GeneratedErrorResponseProcessor>(NULL)) {
    
        if (access(file_path.c_str(), F_OK) == -1) {
            LOG(DEBUG) << "Requested file not found: " << file_path;
            err_response_processor_ = utils::unique_ptr<GeneratedErrorResponseProcessor>(
                new GeneratedErrorResponseProcessor(response_rdy_cb_, http::HTTP_NOT_FOUND));
            return;
        } else if (access(file_path.c_str(), X_OK) == -1) {
            LOG(DEBUG) << "Requested file is not executable: " << file_path;
            err_response_processor_ = utils::unique_ptr<GeneratedErrorResponseProcessor>(
                new GeneratedErrorResponseProcessor(response_rdy_cb_, http::HTTP_NOT_FOUND)); // return another error
            return;
        }
    }
    ~CGIProcessor() {};

  private:
    std::vector<const char*>    envv_;
    utils::unique_ptr<GeneratedErrorResponseProcessor> err_response_processor_;
};

#endif  // WS_SERVER_RESPONSE_PROCESSORS_CGI_PROCESSOR_H
