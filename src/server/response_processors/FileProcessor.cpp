#include "FileProcessor.h"

#include <Request.h>
#include <ResponseCodes.h>
#include <file_utils.h>
#include <unique_ptr.h>

#include <fstream>
#include <stdexcept>

#include "../utils/utils.h"
#include "DirectoryProcessor.h"
#include "Location.h"
#include "RedirectProcessor.h"

FileProcessor::FileProcessor(RequestDestination dest,
                             utils::unique_ptr<http::IResponseCallback> response_rdy_cb,
                             const http::Request& rq)
    : AResponseProcessor(dest, response_rdy_cb)
{
    switch (rq.method) {
        case http::HTTP_GET: ProcessGet_(rq); break;
        case http::HTTP_POST: ProcessPost_(); break;
        case http::HTTP_DELETE: ProcessDelete_(); break;
        default: throw std::logic_error("FileProcessor: Unsupported HTTP method"); break;
    }
}

void FileProcessor::ProcessPost_()
{
    LOG(INFO) << "Processing POST request for file: " << dest_.updated_path;
    // todo: rename from /uploadfolder/.file.txt to /uploadfolder/file.txt
    // if (std::rename(rq.body.c_str(), file_path.c_str()) != 0) {
    //     LOG(DEBUG) << "Upload of file " << file_path << " failed:";
    //     DelegateToErrProc(http::HTTP_CONFLICT);
    //     return;
    // }
    if (utils::DoesPathExist(dest_.updated_path.c_str())) {
        std::map<std::string, std::string> hdrs;
        response_rdy_cb_->Call(utils::unique_ptr<http::Response>(
            new http::Response(http::HTTP_CREATED, http::HTTP_1_1, hdrs, std::vector<char>())));
        return;
    }
    DelegateToErrProc(http::HTTP_INTERNAL_SERVER_ERROR);
}

void FileProcessor::ProcessDelete_()
{
    LOG(INFO) << "Processing DELETE request for file: " << dest_.updated_path;
    if (std::remove(dest_.updated_path.c_str()) != 0) {
        DelegateToErrProc(http::HTTP_NOT_FOUND);
        return;
    }
    std::map<std::string, std::string> hdrs;
    response_rdy_cb_->Call(utils::unique_ptr<http::Response>(
        new http::Response(http::HTTP_NO_CONTENT, http::HTTP_1_1, hdrs, std::vector<char>())));
}

void FileProcessor::ProcessGet_(const http::Request& rq)
{
    LOG(INFO) << "Processing GET request for file: " << dest_.updated_path;
    const Location& loc = *dest_.loc;
    if (utils::IsDirectory(dest_.updated_path.c_str())) {
        if (loc.default_files().size() > 0) {
            for (size_t i = 0; i < loc.default_files().size(); i++) {
                std::string default_file = dest_.updated_path + loc.default_files()[i];
                if (utils::DoesPathExist(default_file.c_str()) &&
                    utils::IsRegularFile(default_file.c_str())) {
                    // delegated_processor_.reset(
                    //     new RedirectProcessor(dest, response_rdy_cb_, http::HTTP_FOUND,
                    //                           rq.rqTarget.path() + loc.default_files()[i]));
                    dest_.updated_path = default_file;
                    ProcessGet_(rq);
                    return;
                }
            }
        }
        if (loc.dir_listing()) {
            delegated_processor_.reset(new DirectoryProcessor(dest_, response_rdy_cb_, rq));
            return;
        }
        DelegateToErrProc(http::HTTP_FORBIDDEN);
        return;
    }
    if (!utils::IsRegularFile(dest_.updated_path.c_str()) ||
        !utils::IsReadable(dest_.updated_path.c_str())) {
        LOG(DEBUG) << "Requested file not found: " << dest_.updated_path;
        DelegateToErrProc(http::HTTP_NOT_FOUND);
        return;
    }
    // std::ifstream file(dest_.updated_path.c_str(), std::ios::binary);
    // if (!file.is_open()) {
    //     LOG(ERROR) << "Requested file cannot be opened: " << dest_.updated_path;
    //     DelegateToErrProc(http::HTTP_INTERNAL_SERVER_ERROR);
    //     return;
    // }
    // std::vector<char> body =
    //     std::vector<char>(std::istreambuf_iterator<char>(file),
    //     std::istreambuf_iterator<char>());
    std::map<std::string, std::string> hdrs;
    hdrs["Content-Type"] = GetContentType_(dest_.updated_path);
    // hdrs["Connection"] = "Close";
    hdrs["Content-Length"] = utils::GetFileSize(dest_.updated_path.c_str());
    response_rdy_cb_->Call(utils::unique_ptr<http::Response>(
        new http::Response(http::HTTP_OK, http::HTTP_1_1, hdrs, dest_.updated_path)));
}

std::string FileProcessor::GetContentType_(const std::string& file)
{
    size_t dot_pos = file.find_last_of('.');
    if (dot_pos == std::string::npos) {
        return utils::kDefaultContentType();
    }
    const char* extension = file.c_str() + dot_pos;
    return utils::GetTypeByExt(extension);
}
