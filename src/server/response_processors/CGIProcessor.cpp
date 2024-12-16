#include "CGIProcessor.h"

#include <EventManager.h>
#include <http.h>
#include <str_utils.h>
#include <sys/socket.h>

bool CGIProcessor::IsValidMethod_(utils::shared_ptr<Location> loc, const http::Request& rq)
{
    if (rq.method == http::HTTP_DELETE) {
        return false;
    }
    for (size_t i = 0; i < loc->allowed_methods().size(); i++) {
        if (rq.method == loc->allowed_methods()[i]) {
            return true;
        }
    }
    return false;
}

std::string CGIProcessor::GetInterpreterByExt_(utils::shared_ptr<Location> loc,
                                               const std::string& script_path)
{
    if (script_path.find_last_of(".") == std::string::npos) {
        return std::string();
    }
    std::string ext = script_path.substr(script_path.find_last_of("."));
    if (std::find(loc->cgi_extensions().begin(), loc->cgi_extensions().end(), ext) ==
        loc->cgi_extensions().end()) {
        return std::string();
    }
    if (ext == ".py") {
        return "/usr/bin/python3";
    } else if (ext == ".php") {
        return "/usr/bin/php";
    } else if (ext == ".pl") {
        return "/usr/bin/perl";
    } else if (ext == ".sh") {
        return "/bin/sh";
    }
    return std::string();
}

void CGIProcessor::SetEnv_(const std::string& script_path, const http::Request& rq)
{
    env_.push_back("GATEWAY_INTERFACE=CGI/1.1");
    env_.push_back("SERVER_SOFTWARE=webserv/1.0");

    env_.push_back("SERVER_PROTOCOL=" + std::string(HttpVerToStr(rq.version).second));
    env_.push_back("SCRIPT_NAME=" + rq.rqTarget.path());
    env_.push_back("REQUEST_METHOD=" + std::string(HttpMethodToStr(rq.method).second));

    if (rq.has_body) {
        // env_.push_back("CONTENT_LENGTH=" + utils::NumericToString(rq.body.size()));
        env_.push_back("CONTENT_TYPE=" + rq.GetHeaderVal("Content-Type").second);
    }
    env_.push_back("QUERY_STRING=" + rq.rqTarget.query());
    // env_.push_back("REMOTE_ADDR=" + utils::IPaddr);  TODO: pass Client Socket IP address here
    // env_.push_back("SERVER_PORT=" + utils::port);  TODO: pass Master Socket port here
    env_.push_back("REMOTE_HOST=" + rq.GetHeaderVal("Host").second);
    env_.push_back("SERVER_NAME=" + rq.GetHeaderVal("Host").second);

    if (rq.GetHeaderVal("Authorization").first) {
        env_.push_back("AUTH_TYPE=" + rq.GetHeaderVal("Authorization").second);
    }

    env_.push_back("PATH_INFO=" + script_path);
    if (rq.GetHeaderVal("Authorization").first) {
        env_.push_back("ACCEPT=" + rq.GetHeaderVal("Accept").second);
    }
    if (rq.GetHeaderVal("Accept").first) {
        env_.push_back("ACCEPT=" + rq.GetHeaderVal("Accept").second);
    }
    env_.push_back("PATH_TRANSLATED=" + script_path);  // same as PATH_INFO ?
    // env_.push_back("HTTP_COOKIE=" + rq.GetHeaderVal("Cookie").second); bonuses
}

int CGIProcessor::Execute_(const std::string& script_path, const http::Request& rq)
{
    int socket_fds[2];
    int status = 0;
    int rq_body_fd;

    if (socketpair(AF_UNIX, SOCK_STREAM, 0, socket_fds) < 0) {
        std::perror("socketpair failed");
        return 1;
    }

    if (rq.has_body) {
        rq_body_fd = open(rq.body, R_OK);
        if (rq_body_fd != 0) {
            std::perror("open failed");
            close(socket_fds[0]);
            close(socket_fds[1]);
            return 1;
        }
    }
    // utils::unique_ptr<c_api::SocketWrapper> client_socket =
    //     utils::unique_ptr<c_api::SocketWrapper>(new c_api::SocketWrapper(sv[0]));
    // utils::unique_ptr<c_api::SocketWrapper> server_socket =
    //     utils::unique_ptr<c_api::SocketWrapper>(new c_api::SocketWrapper(sv[1]));

    pid_t child_pid = fork();
    if (child_pid < 0) {
        std::perror("fork failed");
        return 1;
    } else if (child_pid == 0) {
        SetEnv_(script_path, rq);
        std::vector<char*> env;
        for (size_t i = 0; i < env_.size(); i++) {
            env.push_back(const_cast<char*>(env_[i].c_str()));
        }
        env.push_back(NULL);

        if (dup2(rq_body_fd, STDIN_FILENO) < 0 || dup2(socket_fds[1], STDOUT_FILENO) < 0) {
            std::perror("dup2 failed");
            close(socket_fds[0]);
            close(socket_fds[1]);
            close(rq_body_fd);
            exit(EXIT_FAILURE);
        }

        std::vector<char*> args;
        args.push_back(const_cast<char*>(interpreter_.c_str()));
        args.push_back(const_cast<char*>(script_path.c_str()));
        args.push_back(NULL);

        LOG(DEBUG) << "Executing CGI script: " << args[1];
        close(socket_fds[0]);
        close(rq_body_fd);
        int res = execve(interpreter_.c_str(), args.data(), env.data());
        if (res < 0) {
            std::perror("execve failed");
            exit(res);
        }
    } else {
        close(rq_body_fd);
        close(socket_fds[1]);
        if (waitpid(child_pid, &status, WNOHANG) < 0) {
            std::perror("waitpid failed");
            close(socket_fds[0]);
            close(socket_fds[1]);
            return 1;
        } else if (WIFEXITED(status)) {
            LOG(ERROR) << "Child process exited with " << WEXITSTATUS(status);
            return WEXITSTATUS(status);
        } else if (WIFSIGNALED(status)) {
            LOG(ERROR) << "Child process exited with signal " << WTERMSIG(status);
            return WTERMSIG(status);
        }
    }

    wrapped_socket_ =
        utils::unique_ptr<c_api::SocketWrapper>(new c_api::SocketWrapper(socket_fds[0]));

    if (c_api::EventManager::get().RegisterCallback(
            wrapped_socket_->sockfd(), c_api::CT_READ,
            utils::unique_ptr<c_api::ICallback>(new CGIReadCallback(*wrapped_socket_, buffer_))) !=
        0) {
        LOG(ERROR) << "Could not register CGI read callback";
        return 1;
    }
    LOG(INFO) << "Returning " << buffer_.size() << " bytes from CGI script";
    c_api::EventManager::get().DeleteCallback(wrapped_socket_->sockfd(), c_api::CT_READ);
    c_api::EventManager::get().CheckOnce();
    return status;
}

CGIProcessor::CGIProcessor(const std::string& script_path, const http::Request& rq,
                           utils::shared_ptr<Location> loc,
                           utils::unique_ptr<http::IResponseCallback> response_rdy_cb)
    : AResponseProcessor(response_rdy_cb)
{
    if (!IsValidMethod_(loc, rq)) {
        err_response_processor_ = utils::unique_ptr<AResponseProcessor>(
            new GeneratedErrorResponseProcessor(response_rdy_cb_, http::HTTP_METHOD_NOT_ALLOWED));
        return;
    }

    interpreter_ = GetInterpreterByExt_(loc, script_path);
    if (interpreter_.empty()) {
        LOG(ERROR) << "CGI script not supported: " << script_path;
        err_response_processor_ = utils::unique_ptr<AResponseProcessor>(
            new GeneratedErrorResponseProcessor(response_rdy_cb_, http::HTTP_FORBIDDEN));
        return;
    } else if (access(interpreter_.c_str(), X_OK) == -1) {
        LOG(ERROR) << "CGI script cannot be executed: " << script_path;
        err_response_processor_ =
            utils::unique_ptr<AResponseProcessor>(new GeneratedErrorResponseProcessor(
                response_rdy_cb_, http::HTTP_INTERNAL_SERVER_ERROR));
        return;
    }
    if (access(script_path.c_str(), F_OK | R_OK) == -1) {
        LOG(ERROR) << "Requested CGI script not found: " << script_path;
        err_response_processor_ = utils::unique_ptr<AResponseProcessor>(
            new GeneratedErrorResponseProcessor(response_rdy_cb_, http::HTTP_FORBIDDEN));
        return;
    }

    if (Execute_(script_path, rq)) {
        LOG(ERROR) << "CGI script execution failed: script path: " << script_path;
        err_response_processor_ =
            utils::unique_ptr<AResponseProcessor>(new GeneratedErrorResponseProcessor(
                response_rdy_cb_, http::HTTP_INTERNAL_SERVER_ERROR));
        return;
    }
    response_rdy_cb_->Call(utils::unique_ptr<http::Response>(new http::Response(
        http::HTTP_OK, http::HTTP_1_1, std::map<std::string, std::string>(), buffer_)));
}

CGIProcessor::~CGIProcessor()
{}

CGIProcessor::CGIReadCallback::CGIReadCallback(c_api::SocketWrapper& socket,
                                               std::vector<char>& buffer)
    : socket_(socket), buffer_(buffer)
{}

void CGIProcessor::CGIReadCallback::Call(int /*fd*/)
{
    LOG(DEBUG) << "CGIReadCallback::Call";

    size_t old_buf_size = buffer_.size();
    buffer_.resize(buffer_.size() + 1000);
    LOG(DEBUG) << "CGIReadCallback::Call buffer_.size(): " << buffer_.size();
    size_t bytes_recvd = socket_.Recv(buffer_, 1000);
    LOG(DEBUG) << "CGIReadCallback::Call bytes_recvd: " << bytes_recvd;
    if (bytes_recvd <= 0) {
        LOG(DEBUG) << "nothing read from cgi";
        return;
    }
    if (buffer_.size() > old_buf_size + bytes_recvd) {
        buffer_.resize(old_buf_size + bytes_recvd);
    }
}

// CGIProcessor::ChildProcess::ChildProcess(pid_t pid)
//     : child_pid_(pid), start_time_(std::time(NULL)), status_(0){};


// void CGIProcessor::ChildProcess::Monitor()
// {
//     int timeout = 5;

//     while (true) {
//         if (std::time(NULL) - start_time_ > timeout) {
//             LOG(ERROR) << "Timeout exceeded. Terminating child process...";
//             kill(child_pid_, SIGKILL);
//         }
//         if (waitpid(child_pid_, &status_, WNOHANG) > 0) {
//             if (WIFEXITED(status_)) {
//                 LOG(ERROR) << "Child process exited with status " << WEXITSTATUS(status_);
//             } else if (WIFSIGNALED(status_)) {
//                 LOG(ERROR) << "Child process terminated by signal " << WTERMSIG(status_);
//             }
//             break;
//         }
//     }
// }

// int CGIProcessor::ChildProcess::status() const
// {
//     return status_;
// }
