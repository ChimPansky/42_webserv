#include "FileProcessor.h"

#include <ResponseCodes.h>
#include <file_utils.h>
#include <unique_ptr.h>
#include <unistd.h>

#include <fstream>
#include <stdexcept>

#include "utils/utils.h"

FileProcessor::FileProcessor(const Server& server, const std::string& file_path,
                             utils::unique_ptr<http::IResponseCallback> response_rdy_cb,
                             const http::Request& rq)
    : AResponseProcessor(server, response_rdy_cb)
{
    switch (rq.method) {
        case http::HTTP_GET: ProcessGet_(file_path); break;
        case http::HTTP_POST: ProcessPost_(file_path); break;
        case http::HTTP_DELETE: ProcessDelete_(file_path); break;
        default: throw std::logic_error("FileProcessor: Unsupported HTTP method"); break;
    }
}

void FileProcessor::ProcessPost_(const std::string& file_path)
{
    LOG(INFO) << "Processing POST request for file: " << file_path;
    DelegateToErrProc(http::HTTP_NOT_IMPLEMENTED);
}

void FileProcessor::ProcessDelete_(const std::string& file_path)
{
    LOG(INFO) << "Processing DELETE request for file: " << file_path;
    DelegateToErrProc(http::HTTP_NOT_IMPLEMENTED);
}

void FileProcessor::ProcessGet_(const std::string& file_path)
{
    if (!utils::DoesPathExist(file_path.c_str())) {
        LOG(DEBUG) << "Requested file not found: " << file_path;
        DelegateToErrProc(http::HTTP_NOT_FOUND);
        return;
    }
    std::ifstream file(file_path.c_str(), std::ios::binary);
    if (!file.is_open()) {
        LOG(DEBUG) << "Requested file cannot be opened: " << file_path;
        DelegateToErrProc(http::HTTP_INTERNAL_SERVER_ERROR);
        return;
    }
    std::vector<char> body =
        std::vector<char>(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
    std::map<std::string, std::string> hdrs;
    hdrs["Content-Type"] = GetContentType_(file_path);
    // hdrs["Connection"] = "Close";
    hdrs["Content-Length"] = utils::NumericToString(body.size());
    response_rdy_cb_->Call(utils::unique_ptr<http::Response>(
        new http::Response(http::HTTP_OK, http::HTTP_1_1, hdrs, body)));
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
