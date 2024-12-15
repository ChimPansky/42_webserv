#include "FileProcessor.h"

#include <ResponseCodes.h>
#include <file_utils.h>
#include <unique_ptr.h>
#include <unistd.h>

#include <fstream>

#include "utils/utils.h"

FileProcessor::FileProcessor(const std::string& file_path,
                             utils::unique_ptr<http::IResponseCallback> response_rdy_cb)
    : AResponseProcessor(response_rdy_cb)
{
    if (!utils::DoesPathExist(file_path.c_str())) {
        LOG(DEBUG) << "Requested file not found: " << file_path;
        err_response_processor_ = utils::unique_ptr<AResponseProcessor>(
            new GeneratedErrorResponseProcessor(response_rdy_cb_, http::HTTP_NOT_FOUND));
        return;
    }
    // TODO if directory delegate to DirectoryProcessor or 404
    if (utils::IsDirectory(file_path.c_str())) {
        LOG(DEBUG) << "Requested file is a directory: " << file_path;
        err_response_processor_ = utils::unique_ptr<GeneratedErrorResponseProcessor>(
            new GeneratedErrorResponseProcessor(response_rdy_cb_, http::HTTP_NOT_FOUND));
        return;
    }
    // check if POST/GET/DELETE
    std::ifstream file(file_path.c_str(), std::ios::binary);
    if (!file.is_open()) {
        LOG(DEBUG) << "Requested file cannot be opened: " << file_path;
        err_response_processor_ =
            utils::unique_ptr<AResponseProcessor>(new GeneratedErrorResponseProcessor(
                response_rdy_cb_, http::HTTP_INTERNAL_SERVER_ERROR));
        return;
    }
    std::vector<char> body =
        std::vector<char>(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
    std::map<std::string, std::string> hdrs;
    hdrs["Content-Type"] = GetContentType(file_path);
    // hdrs["Connection"] = "Closed";
    hdrs["Content-Length"] = utils::NumericToString(body.size());
    response_rdy_cb_->Call(utils::unique_ptr<http::Response>(
        new http::Response(http::HTTP_OK, http::HTTP_1_1, hdrs, body)));
}

std::string FileProcessor::GetContentType(const std::string& file)
{
    size_t dot_pos = file.find_last_of('.');
    if (dot_pos == std::string::npos) {
        return utils::kDefaultContentType();
    }
    const char* extension = file.c_str() + dot_pos;
    return utils::GetTypeByExt(extension);
}
