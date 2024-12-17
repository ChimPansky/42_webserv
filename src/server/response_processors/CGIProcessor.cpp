#include "CGIProcessor.h"

#include <EventManager.h>
#include <cgi/cgi.h>
#include <file_utils.h>
#include <http.h>
#include <str_utils.h>
#include <sys/socket.h>
#include <sys/wait.h>

#include <cctype>
#include <cstring>

#include "../utils/utils.h"
#include "ErrorProcessor.h"

bool IsValidExtension(const std::string& filename, const std::vector<std::string>& allowed_exts)
{
    for (size_t i = 0; i < allowed_exts.size(); i++) {
        if (utils::fs::CheckFileExtension(filename, allowed_exts[i])) {
            return true;
        }
    }
    return false;
}

CGIProcessor::CGIProcessor(const Server& server, const std::string& script_path,
                           const http::Request& rq, utils::shared_ptr<Location> loc,
                           utils::unique_ptr<http::IResponseCallback> response_rdy_cb)
    : AResponseProcessor(server, response_rdy_cb)
{
    if (!IsValidMethod_(loc, rq)) {
        LOG(ERROR) << "Method " << http::HttpMethodToStr(rq.method).second << " isn't allowed";
        delegated_processor_ = utils::unique_ptr<AResponseProcessor>(
            new ErrorProcessor(server_, response_rdy_cb_, http::HTTP_METHOD_NOT_ALLOWED));
        return;
    }
    if (!IsValidExtension(script_path, loc->cgi_extensions())) {
        LOG(ERROR) << "CGI script not supported: " << script_path;
        delegated_processor_ = utils::unique_ptr<AResponseProcessor>(
            new ErrorProcessor(server_, response_rdy_cb_, http::HTTP_NOT_IMPLEMENTED));
        return;
    }

    std::string interpreter = utils::GetInterpreterByExt(script_path);
    if (!utils::IsReadable(script_path.c_str()) || !utils::IsExecutable(interpreter.c_str())) {
        LOG(ERROR) << "CGI script cannot be executed: " << script_path;
        delegated_processor_ = utils::unique_ptr<AResponseProcessor>(
            new ErrorProcessor(server_, response_rdy_cb_, http::HTTP_INTERNAL_SERVER_ERROR));
        return;
    }

    if (Execute_(script_path, interpreter, rq)) {
        LOG(ERROR) << "CGI script execution failed: script path: " << script_path;
        delegated_processor_ = utils::unique_ptr<AResponseProcessor>(
            new ErrorProcessor(server_, response_rdy_cb_, http::HTTP_INTERNAL_SERVER_ERROR));
        return;
    }
}

CGIProcessor::~CGIProcessor()
{
    c_api::EventManager::get().DeleteCallback(parent_socket_->sockfd());
}

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

// child process after fork
void SetUpChild(int socket_fd, const std::string& script_path, const std::string& interpreter,
                const http::Request& rq)
{
    // todo file fd to wrapper to close automatically
    int rq_body_fd = 0;

    if (rq.has_body) {
        rq_body_fd = open(rq.body, O_RDONLY);
        if (rq_body_fd < 0) {
            std::perror("open failed");
            close(socket_fd);
            exit(EXIT_FAILURE);
        }
    }
    utils::unique_ptr<c_api::SocketWrapper> rq_body =
        utils::unique_ptr<c_api::SocketWrapper>(new c_api::SocketWrapper(rq_body_fd));

    if (dup2(rq_body_fd, STDIN_FILENO) < 0 || dup2(socket_fd, STDOUT_FILENO) < 0) {
        std::perror("dup2 failed");
        close(socket_fd);
        exit(EXIT_FAILURE);
    }
    LOG(ERROR) << "Executing CGI script: " << script_path;

    std::vector<std::string> string_env = cgi::GetEnv(script_path, rq);
    std::vector<char*> env;
    for (size_t i = 0; i < string_env.size(); i++) {
        env.push_back(const_cast<char*>(string_env[i].c_str()));
    }
    env.push_back(NULL);

    std::vector<char*> args;
    args.push_back(const_cast<char*>(interpreter.c_str()));
    args.push_back(const_cast<char*>(script_path.c_str()));
    args.push_back(NULL);

    execve(interpreter.c_str(), args.data(), env.data());
    LOG(ERROR) << "CGI failed with error " << std::strerror(errno);
    close(socket_fd);
    exit(EXIT_FAILURE);
}

int CGIProcessor::Execute_(const std::string& script_path, const std::string& interpreter,
                           const http::Request& rq)
{
    int status = 0;

    std::pair<utils::unique_ptr<c_api::SocketWrapper>, utils::unique_ptr<c_api::SocketWrapper> >
        socket_pair = c_api::SocketWrapper::CreateSocketPair();
    utils::unique_ptr<c_api::SocketWrapper> child_socket;
    if (socket_pair.first) {
        parent_socket_ = socket_pair.first;
        child_socket = socket_pair.second;
    } else {
        LOG(DEBUG) << "Failed to create a socket_pair";
        return 1;
    }
    if (c_api::EventManager::get().RegisterCallback(
            parent_socket_->sockfd(), c_api::CT_READ,
            utils::unique_ptr<c_api::ICallback>(new ReadChildOutputCallback(*this))) != 0) {
        LOG(ERROR) << "Could not register CGI read callback";
        return 1;
    }

    pid_t child_pid = fork();
    if (child_pid < 0) {
        std::perror("fork failed");
        return 1;
    } else if (child_pid == 0) {
        parent_socket_.reset();
        SetUpChild(child_socket->sockfd(), script_path, interpreter, rq);
    }
    // child_socket.reset();
    return status;
}

CGIProcessor::ReadChildOutputCallback::ReadChildOutputCallback(CGIProcessor& processor)
    : processor_(processor)
{}

void CGIProcessor::ReadChildOutputCallback::Call(int fd)
{
    LOG(DEBUG) << "ReadChildOutputCallback::Call with " << fd;

    size_t old_buf_size = processor_.cgi_out_buffer_.size();
    processor_.cgi_out_buffer_.resize(processor_.cgi_out_buffer_.size() + 1000);
    LOG(DEBUG) << "ReadChildOutputCallback::Call processor_.cgi_out_buffer_.size(): "
               << processor_.cgi_out_buffer_.size();
    ssize_t bytes_recvd = processor_.parent_socket_->Recv(processor_.cgi_out_buffer_, 1000);
    LOG(DEBUG) << "ReadChildOutputCallback::Call bytes_recvd: " << bytes_recvd;
    if (bytes_recvd < 0) {
        LOG(ERROR) << "error on recv" << std::strerror(errno);
        return;
    } else if (bytes_recvd == 0) {
        processor_.cgi_out_buffer_.resize(old_buf_size);
        LOG(INFO) << "Returning " << processor_.cgi_out_buffer_.size() << " bytes from CGI script";
        c_api::EventManager::get().DeleteCallback(processor_.parent_socket_->sockfd());
        std::pair<bool, utils::unique_ptr<http::Response> > rs =
            cgi::ParseCgiResponse(processor_.cgi_out_buffer_);
        if (!rs.first) {
            processor_.delegated_processor_ = utils::unique_ptr<AResponseProcessor>(
                new ErrorProcessor(processor_.server_, processor_.response_rdy_cb_,
                                   http::HTTP_INTERNAL_SERVER_ERROR));
            return;
        } else {
            processor_.response_rdy_cb_->Call(rs.second);
        }
    }
    if (processor_.cgi_out_buffer_.size() > old_buf_size + bytes_recvd) {
        processor_.cgi_out_buffer_.resize(old_buf_size + bytes_recvd);
    }
}
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
