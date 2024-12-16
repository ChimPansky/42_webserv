#include "CGIProcessor.h"

#include <EventManager.h>
#include <http.h>
#include <str_utils.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <utils/utils.h>

bool IsValidExtension(const std::string& filename, const std::vector<std::string>& allowed_exts)
{
    for (size_t i = 0; i < allowed_exts.size(); i++) {
        if (utils::fs::CheckFileExtension(filename, allowed_exts[i])) {
            return true;
        }
    }
    return false;
}


CGIProcessor::CGIProcessor(const std::string& script_path, const http::Request& rq,
                           utils::shared_ptr<Location> loc,
                           utils::unique_ptr<http::IResponseCallback> response_rdy_cb)
    : AResponseProcessor(response_rdy_cb)
{
    std::string interpreter;

    if (!IsValidExtension(script_path, loc->cgi_extensions())) {
        LOG(ERROR) << "CGI script not supported: " << script_path;
        err_response_processor_ = utils::unique_ptr<AResponseProcessor>(
            new GeneratedErrorResponseProcessor(response_rdy_cb_, http::HTTP_NOT_IMPLEMENTED));
        return;
    }

    interpreter = utils::GetInterpreterByExt_(script_path);
    if (!utils::fs::IsReadable(script_path) || !utils::fs::IsExecutable(script_path)) {
        LOG(ERROR) << "CGI script cannot be executed: " << script_path;
        err_response_processor_ =
            utils::unique_ptr<AResponseProcessor>(new GeneratedErrorResponseProcessor(
                response_rdy_cb_, http::HTTP_INTERNAL_SERVER_ERROR));
        return;
    }

    if (Execute_(script_path, interpreter, rq)) {
        LOG(ERROR) << "CGI script execution failed: script path: " << script_path;
        err_response_processor_ =
            utils::unique_ptr<AResponseProcessor>(new GeneratedErrorResponseProcessor(
                response_rdy_cb_, http::HTTP_INTERNAL_SERVER_ERROR));
        return;
    }
}

CGIProcessor::~CGIProcessor()
{
    c_api::EventManager::get().DeleteCallback(wrapped_socket_->sockfd());
}

// bool CGIProcessor::IsValidMethod_(utils::shared_ptr<Location> loc, const http::Request& rq)
// {
//     if (rq.method == http::HTTP_DELETE) {
//         return false;
//     }
//     for (size_t i = 0; i < loc->allowed_methods().size(); i++) {
//         if (rq.method == loc->allowed_methods()[i]) {
//             return true;
//         }
//     }
//     return false;
// }

std::vector<std::string> SetEnv(const std::string& script_path, const http::Request& rq)
{
    std::vector<std::string> env;

    env.push_back("GATEWAY_INTERFACE=CGI/1.1");
    env.push_back("SERVER_SOFTWARE=webserv/1.0");

    env.push_back("SERVER_PROTOCOL=" + std::string(HttpVerToStr(rq.version).second));
    env.push_back("SCRIPT_NAME=" + rq.rqTarget.path());
    env.push_back("REQUEST_METHOD=" + std::string(HttpMethodToStr(rq.method).second));

    if (rq.has_body) {
        env.push_back("CONTENT_LENGTH=" + utils::NumericToString(std::string(rq.body).size()));
        env.push_back("CONTENT_TYPE=" + rq.GetHeaderVal("Content-Type").second);
    }
    env.push_back("QUERY_STRING=" + rq.rqTarget.query());
    // env.push_back("REMOTE_ADDR=" + utils::IPaddr);  TODO: pass Client Socket IP address here
    // env.push_back("SERVER_PORT=" + utils::port);  TODO: pass Master Socket port here
    env.push_back("REMOTE_HOST=" + rq.GetHeaderVal("Host").second);
    env.push_back("SERVER_NAME=" + rq.GetHeaderVal("Host").second);

    if (rq.GetHeaderVal("Authorization").first) {
        env.push_back("AUTH_TYPE=" + rq.GetHeaderVal("Authorization").second);
    }

    env.push_back("PATH_INFO=" + script_path);
    if (rq.GetHeaderVal("Authorization").first) {
        env.push_back("ACCEPT=" + rq.GetHeaderVal("Accept").second);
    }
    if (rq.GetHeaderVal("Accept").first) {
        env.push_back("ACCEPT=" + rq.GetHeaderVal("Accept").second);
    }
    env.push_back("PATH_TRANSLATED=" + script_path);
    // env_.push_back("HTTP_COOKIE=" + rq.GetHeaderVal("Cookie").second); bonuses
    return env;
}

// child process after fork
void SetUpChild(int socket_fd, const std::string& script_path, const std::string& interpreter,
                const http::Request& rq)
{
    // todo file fd to wrapper to close automatically
    int rq_body_fd = 0;

    LOG(INFO) << "Executing CGI script: " << script_path;
    LOG(WARNING) << "SOCK FD from child: " << socket_fd;

    if (rq.has_body) {
        rq_body_fd = open(rq.body, O_RDONLY);
        LOG(WARNING) << "opened file " << rq.body << " fd: " << rq_body_fd;
        if (rq_body_fd < 0) {
            std::perror("open failed");
            close(socket_fd);
            exit(EXIT_FAILURE);
        }
    }

    if (dup2(rq_body_fd, STDIN_FILENO) < 0 || dup2(socket_fd, STDOUT_FILENO) < 0) {
        LOG(WARNING) << "dup 2 failed ";
        std::perror("dup2 failed");
        close(socket_fd);
        close(rq_body_fd);
        exit(EXIT_FAILURE);
    }

    std::vector<std::string> string_env = SetEnv(script_path, rq);
    std::vector<char*> env;
    for (size_t i = 0; i < string_env.size(); i++) {
        env.push_back(const_cast<char*>(string_env[i].c_str()));
    }
    env.push_back(NULL);

    std::vector<char*> args;
    args.push_back(const_cast<char*>(interpreter.c_str()));
    args.push_back(const_cast<char*>(script_path.c_str()));
    args.push_back(NULL);

    sleep(1000);
    execve(interpreter.c_str(), args.data(), env.data());
    LOG(ERROR) << "CGI failed: ";
    // change to streerror
    std::perror("execve failed");
    close(socket_fd);
    close(rq_body_fd);
    exit(EXIT_FAILURE);
}

int CGIProcessor::Execute_(const std::string& script_path, const std::string& interpreter,
                           const http::Request& rq)
{
    const int kChildSocket = 1, kParentSocket = 0;
    int status = 0;
    int socket_fds[2];

    if (socketpair(AF_UNIX, SOCK_STREAM, 0, socket_fds) < 0) {
        std::perror("socketpair failed");
        return 1;
    }

    wrapped_socket_.reset(new c_api::SocketWrapper(socket_fds[kParentSocket]));
    LOG(WARNING) << "wrapped SOCK FD: " << wrapped_socket_->sockfd();
    if (c_api::EventManager::get().RegisterCallback(
            wrapped_socket_->sockfd(), c_api::CT_READ,
            utils::unique_ptr<c_api::ICallback>(new ReadChildOutputCallback(*this))) != 0) {
        LOG(ERROR) << "Could not register CGI read callback";
        return 1;
    }

    pid_t child_pid = fork();
    if (child_pid < 0) {
        close(socket_fds[kChildSocket]);
        std::perror("fork failed");
        return 1;
    } else if (child_pid == 0) {
        close(socket_fds[kParentSocket]);
        SetUpChild(socket_fds[kChildSocket], script_path, interpreter, rq);
    }
    close(socket_fds[kChildSocket]);

    // close(socket_fds[1]);
    // if (waitpid(child_pid, &status, 0) < 0) {
    //     std::perror("waitpid failed");
    //     close(socket_fds[0]);
    //     close(socket_fds[1]);
    //     if (rq_body_fd) {
    //         close(rq_body_fd);
    //     }
    //     return 1;
    // }
    /*  else if (WIFEXITED(status)) {
        LOG(ERROR) << "Child process exited with " << WEXITSTATUS(status);
        return WEXITSTATUS(status);
    } else if (WIFSIGNALED(status)) {
        LOG(ERROR) << "Child process exited with signal " << WTERMSIG(status);
        return WTERMSIG(status);
    } */

    // wrapped_socket_.reset(new c_api::SocketWrapper(socket_fd));
    // LOG(WARNING) << "wrapped SOCK FD: " << wrapped_socket_->sockfd();
    // if (c_api::EventManager::get().RegisterCallback(
    //         wrapped_socket_->sockfd(), c_api::CT_READ,
    //         utils::unique_ptr<c_api::ICallback>(new ReadChildOutputCallback(*this))) !=
    //     0) {
    //     LOG(ERROR) << "Could not register CGI read callback";
    //     return 1;
    // }
    return status;
}


CGIProcessor::ReadChildOutputCallback::ReadChildOutputCallback(CGIProcessor& processor)
    : processor_(processor)
{}

void CGIProcessor::ReadChildOutputCallback::Call(int fd)
{
    LOG(DEBUG) << "ReadChildOutputCallback::Call with " << fd;

    size_t old_buf_size = processor_.buffer_.size();
    processor_.buffer_.resize(processor_.buffer_.size() + 1000);
    LOG(DEBUG) << "ReadChildOutputCallback::Call processor_.buffer_.size(): "
               << processor_.buffer_.size();
    ssize_t bytes_recvd = processor_.wrapped_socket_->Recv(processor_.buffer_, 1000);
    LOG(DEBUG) << "ReadChildOutputCallback::Call bytes_recvd: " << bytes_recvd;
    if (bytes_recvd < 0) {
        LOG(ERROR) << "error on recv: ";  // << std::strerror(errno);
        return;
    } else if (bytes_recvd == 0) {
        LOG(INFO) << "Returning " << processor_.buffer_.size() << " bytes from CGI script";
        c_api::EventManager::get().DeleteCallback(processor_.wrapped_socket_->sockfd());
        processor_.response_rdy_cb_->Call(utils::unique_ptr<http::Response>(
            new http::Response(http::HTTP_OK, http::HTTP_1_1, std::map<std::string, std::string>(),
                               processor_.buffer_)));
    }
    if (processor_.buffer_.size() > old_buf_size + bytes_recvd) {
        processor_.buffer_.resize(old_buf_size + bytes_recvd);
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
