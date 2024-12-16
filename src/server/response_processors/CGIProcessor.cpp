#include "CGIProcessor.h"

#include <EventManager.h>
#include <http.h>
#include <str_utils.h>
#include <sys/socket.h>
#include <sys/wait.h>

#include <cctype>

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
    std::string interpreter;

    if (!IsValidExtension(script_path, loc->cgi_extensions())) {
        LOG(ERROR) << "CGI script not supported: " << script_path;
        delegated_processor_ = utils::unique_ptr<AResponseProcessor>(
            new ErrorProcessor(server_, response_rdy_cb_, http::HTTP_NOT_IMPLEMENTED));
        return;
    }

    interpreter = utils::GetInterpreterByExt_(script_path);
    if (!utils::fs::IsReadable(script_path) || !utils::fs::IsExecutable(script_path)) {
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
        env.push_back("CONTENT_LENGTH=" + rq.GetHeaderVal("Content-Length").second);
        env.push_back("CONTENT_TYPE=" + rq.GetHeaderVal("Content-Type").second);
    }
    env.push_back("QUERY_STRING=" + rq.rqTarget.query());
    // if (rq.method == http::HTTP_GET) {
    // }
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

    // sleep(1000);
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

    if (socketpair(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK, 0, socket_fds) < 0) {
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

bool IsKeyValid(const std::string& key)
{
    if (key.empty()) {
        return false;
    }
    for (size_t i = 0; i < key.length(); ++i) {
        char c = key[i];
        if (!std::isalnum(c) && c != '-' && c != '_') {
            return false;
        }
    }
    return true;
}

bool IsValValid(const std::string& val)
{
    for (size_t i = 0; i < val.length(); ++i) {
        char c = val[i];
        if (c < ' ' || c > '~') {
            return false;
        }
    }
    return true;
}

bool ExtractHeaderToMap(std::map<std::string, std::string>& headers, std::string rs_line)
{
    size_t separator = rs_line.find(":");
    if (separator == std::string::npos) {
        return false;
    }
    std::string key = rs_line.substr(0, separator);
    if (!IsKeyValid(key)) {
        return false;
    }
    std::string value = rs_line.substr(separator + 1);
    value = utils::fs::Trim(value, " \t");
    if (!IsValValid(value)) {
        return false;
    }
    if (!value.empty()) {
        headers[key] = value;
    }
    return true;
}

std::pair<bool, int> ExtractStatus(const std::string& status_str)
{
    if (status_str.length() < 5) {
        return std::make_pair(false, 0);
    }
    std::pair<bool, int> status_or = utils::StrToNumericNoThrow<int>(status_str.substr(0, 3));
    if (!status_or.first || status_or.second < 100) {
        return std::make_pair(false, 0);
    }
    if ((status_str[4] != ' ' && status_str[4] != '\t') || !std::isalnum(status_str[5])) {
        return std::make_pair(false, 0);
    }
    return std::make_pair(true, status_or.second);
}

std::pair<bool, utils::unique_ptr<http::Response> > ParseCgiResponse(std::vector<char>& buf)
{
    std::pair<bool, utils::unique_ptr<http::Response> > res =
        std::make_pair(false, utils::unique_ptr<http::Response>(NULL));
    if (buf.empty() || buf[0] == '\n') {
        return res;
    }

    std::map<std::string, std::string> headers;

    std::vector<char>::iterator cur_pos = buf.begin();
    while (true) {
        std::vector<char>::iterator line_end = std::find(cur_pos, buf.end(), '\n');
        if (line_end == buf.end()) {
            break;
        }

        size_t line_len = std::distance(cur_pos, line_end);
        if (line_len == 0) {
            ++cur_pos;
            break;
        }
        if (!ExtractHeaderToMap(headers, std::string(cur_pos.base(), line_len))) {
            return res;
        }
        cur_pos = line_end + 1;
    }

    // Retain the binary data in buf
    buf.erase(buf.begin(), cur_pos);
    http::ResponseCode rs_code = http::HTTP_OK;
    if (headers.find("Status") != headers.end()) {
        std::pair<bool, int> parsed_status = ExtractStatus(headers["Status"]);
        if (!parsed_status.first) {
            return res;
        }
        // TODO: add parse func
        rs_code = static_cast<http::ResponseCode>(parsed_status.second);
    }
    if (headers.find("Content-Length") == headers.end() && !buf.empty()) {
        headers["Content-Length"] = utils::NumericToString(buf.size());
    }

    return std::make_pair(true, utils::unique_ptr<http::Response>(
                                    new http::Response(rs_code, http::HTTP_1_1, headers, buf)));
}

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
        processor_.buffer_.resize(old_buf_size);
        LOG(INFO) << "Returning " << processor_.buffer_.size() << " bytes from CGI script";
        c_api::EventManager::get().DeleteCallback(processor_.wrapped_socket_->sockfd());
        std::pair<bool, utils::unique_ptr<http::Response> > rs =
            ParseCgiResponse(processor_.buffer_);
        if (!rs.first) {
            processor_.delegated_processor_ = utils::unique_ptr<AResponseProcessor>(
                new ErrorProcessor(processor_.server_, processor_.response_rdy_cb_,
                                   http::HTTP_INTERNAL_SERVER_ERROR));
            return;
        } else {
            processor_.response_rdy_cb_->Call(rs.second);
        }
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
