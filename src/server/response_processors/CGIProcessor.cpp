#include "CGIProcessor.h"

#include <ChildProcessesManager.h>
#include <EventManager.h>
#include <cgi/cgi.h>
#include <errors.h>
#include <file_utils.h>
#include <http.h>

#include "../utils/utils.h"

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

CGIProcessor::CGIProcessor(RequestDestination dest,
                           utils::unique_ptr<http::IResponseCallback> response_rdy_cb,
                           const http::Request& rq)
    : AResponseProcessor(dest, response_rdy_cb)
{
    if (!IsValidExtension(dest.updated_path, dest.loc->cgi_extensions())) {
        LOG(ERROR) << "CGI extension not supported: " << dest.updated_path;
        DelegateToErrProc(http::HTTP_NOT_IMPLEMENTED);
        return;
    }

    std::string interpreter = utils::GetInterpreterByExt(dest.updated_path);
    if (!utils::IsReadable(dest.updated_path.c_str())) {
        LOG(ERROR) << "CGI script cannot be executed: " << dest.updated_path;
        DelegateToErrProc(http::HTTP_INTERNAL_SERVER_ERROR);
        return;
    }

    c_api::ExecParams exec_params(interpreter, dest.updated_path,
                                  cgi::GetEnv(dest.updated_path, rq), rq.body.path.c_str());
    child_process_description_ = c_api::ChildProcessesManager::get().TryRunChildProcess(
        exec_params, utils::unique_ptr<c_api::IChildDiedCb>(new ChildProcessDoneCb(*this)));
    if (!child_process_description_.ok()) {
        LOG(ERROR) << "Cannot run child process";
        DelegateToErrProc(http::HTTP_INTERNAL_SERVER_ERROR);
        return;
    }
    if (!c_api::EventManager::TryRegisterCallback(
            child_process_description_->sock().sockfd(), c_api::CT_READ,
            utils::unique_ptr<c_api::ICallback>(new ReadChildOutputCallback(*this)))) {
        LOG(ERROR) << "Could not register CGI read callback";
        DelegateToErrProc(http::HTTP_INTERNAL_SERVER_ERROR);
        return;
    }
}

CGIProcessor::~CGIProcessor()
{
    if (child_process_description_) {
        c_api::EventManager::DeleteCallback(child_process_description_->sock().sockfd());
        c_api::ChildProcessesManager::get().KillChildProcess(child_process_description_->pid());
    }
}

void CGIProcessor::ReadChildOutputCallback::Call(int /* fd */)
{
    // LOG(DEBUG) << "ReadChildOutputCallback::Call with " << fd;
    std::vector<char>& buf = processor_.cgi_out_buffer_;
    c_api::RecvPackage pack = processor_.child_process_description_->sock().Recv();
    if (pack.status == c_api::RS_SOCK_ERR) {
        LOG(ERROR) << "Error on recv from child proc: " << utils::GetSystemErrorDescr();
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
    utils::maybe<utils::unique_ptr<http::Response> > rs =
        cgi::ParseCgiResponse(processor_.cgi_out_buffer_);
    if (!rs) {
        processor_.DelegateToErrProc(http::HTTP_INTERNAL_SERVER_ERROR);
        return;
    }
    processor_.response_rdy_cb_->Call(*rs);
}
