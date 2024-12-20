#include "CGIProcessor.h"

#include <ChildProcessesManager.h>
#include <EventManager.h>
#include <cgi/cgi.h>
#include <file_utils.h>
#include <http.h>
#include <sys/socket.h>
#include <sys/wait.h>

#include <cctype>
#include <cstring>

#include "../utils/utils.h"
#include "ErrorProcessor.h"

namespace {

bool IsValidExtension(const std::string& filename, const std::vector<std::string>& allowed_exts)
{
    for (size_t i = 0; i < allowed_exts.size(); i++) {
        if (utils::CheckFileExtension(filename, allowed_exts[i])) {
            return true;
        }
    }
    return false;
}

}  // namespace

CGIProcessor::CGIProcessor(const Server& server, const std::string& script_path,
                           const http::Request& rq,
                           const std::vector<std::string>& allowed_cgi_extensions,
                           utils::unique_ptr<http::IResponseCallback> response_rdy_cb)
    : AResponseProcessor(server, response_rdy_cb)
{
    if (!IsValidExtension(script_path, allowed_cgi_extensions)) {
        LOG(ERROR) << "CGI extension not supported: " << script_path;
        DelegateToErrProc(http::HTTP_NOT_IMPLEMENTED);
        return;
    }

    std::string interpreter = utils::GetInterpreterByExt(script_path);
    if (!utils::IsReadable(script_path.c_str())) {
        LOG(ERROR) << "CGI script cannot be executed: " << script_path;
        DelegateToErrProc(http::HTTP_INTERNAL_SERVER_ERROR);
        return;
    }

    c_api::ExecParams exec_params(interpreter, script_path, cgi::GetEnv(script_path, rq), rq.body);
    std::pair<bool, utils::unique_ptr<c_api::Socket> > res =
        c_api::ChildProcessesManager::get().TryRunChildProcess(
            exec_params, utils::unique_ptr<c_api::IChildDiedCb>(new ChildProcessDoneCb(*this)));
    if (!res.first) {
        LOG(ERROR) << "Cannot run child process";
        DelegateToErrProc(http::HTTP_INTERNAL_SERVER_ERROR);
        return;
    }
    parent_socket_ = res.second;
    if (!c_api::EventManager::TryRegisterCallback(
            parent_socket_->sockfd(), c_api::CT_READ,
            utils::unique_ptr<c_api::ICallback>(new ReadChildOutputCallback(*this)))) {
        LOG(ERROR) << "Could not register CGI read callback";
        DelegateToErrProc(http::HTTP_INTERNAL_SERVER_ERROR);
        return;
    }
}

CGIProcessor::~CGIProcessor()
{
    if (parent_socket_) {
        c_api::EventManager::DeleteCallback(parent_socket_->sockfd());
    }
}

void CGIProcessor::ReadChildOutputCallback::Call(int /* fd */)
{
    // LOG(DEBUG) << "ReadChildOutputCallback::Call with " << fd;
    std::vector<char>& buf = processor_.cgi_out_buffer_;
    c_api::RecvPackage pack = processor_.parent_socket_->Recv();
    if (pack.status == c_api::RS_SOCK_ERR) {
        LOG(ERROR) << "error on recv" << std::strerror(errno);  // TODO: is errno check allowed?
        return;
    } else if (pack.status == c_api::RS_SOCK_CLOSED) {
        LOG(INFO) << "Done reading CGI output, got " << buf.size() << " bytes";
        return;
    } else if (pack.status == c_api::RS_OK) {
        buf.reserve(buf.size() + pack.data_size);
        std::copy(pack.data, pack.data + pack.data_size, std::back_inserter(buf));
    }
}

void CGIProcessor::ChildProcessDoneCb::Call(int child_exit_status)
{
    if (child_exit_status != EXIT_SUCCESS) {
        LOG(ERROR) << "CGI child process failed";
        processor_.DelegateToErrProc(http::HTTP_INTERNAL_SERVER_ERROR);
        return;
    }
    std::pair<bool, utils::unique_ptr<http::Response> > rs =
        cgi::ParseCgiResponse(processor_.cgi_out_buffer_);
    if (!rs.first) {
        processor_.DelegateToErrProc(http::HTTP_INTERNAL_SERVER_ERROR);
        return;
    }
    processor_.response_rdy_cb_->Call(rs.second);
}
