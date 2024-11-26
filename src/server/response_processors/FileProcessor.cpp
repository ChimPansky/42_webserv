#include "FileProcessor.h"

#include <ResponseCodes.h>
#include <fcntl.h>
#include <unique_ptr.h>
#include <unistd.h>

#include <fstream>

#include "utils/utils.h"

FileProcessor::FileProcessor(const std::string& file_path,
                             utils::unique_ptr<http::IResponseCallback> response_rdy_cb)
    : AResponseProcessor(response_rdy_cb),
      err_response_processor_(utils::unique_ptr<GeneratedErrorResponseProcessor>(NULL))
{
    if (access(file_path.c_str(), F_OK) == -1) {
        LOG(DEBUG) << "Requested file not found: " << file_path;
        err_response_processor_ = utils::unique_ptr<GeneratedErrorResponseProcessor>(
            new GeneratedErrorResponseProcessor(response_rdy_cb_, http::HTTP_NOT_FOUND));
        return;
    }
    std::ifstream file(file_path.c_str(), std::ios::binary);
    if (!file.is_open()) {
        LOG(DEBUG) << "Requested file cannot be opened: " << file_path;
        err_response_processor_ =
            utils::unique_ptr<GeneratedErrorResponseProcessor>(new GeneratedErrorResponseProcessor(
                response_rdy_cb_, http::HTTP_INTERNAL_SERVER_ERROR));
        return;
    }
    std::vector<char> body = ReadFile_(file);
    std::map<std::string, std::string> hdrs;
    hdrs["Content-Type"] = GetContentType(file_path);
    // hdrs["Connection"] = "Closed";
    hdrs["Content-Length"] = utils::NumericToString(body.size());
    response_rdy_cb_->Call(utils::unique_ptr<http::Response>(
        new http::Response(http::HTTP_OK, http::HTTP_1_1, hdrs, body)));
}

std::vector<char> FileProcessor::ReadFile_(std::ifstream& file)
{
    return std::vector<char>(std::istreambuf_iterator<char>(file),
                             std::istreambuf_iterator<char>());
}

std::string FileProcessor::GetContentType(const std::string& file)
{
    std::map<std::string, std::string> mime_types = utils::GetMimeTypes();

    size_t dot_pos = file.find_last_of('.');
    if (dot_pos == std::string::npos) {
        return utils::kDefaultContentType();
    }
    std::string extension = file.substr(dot_pos);
    if (mime_types.find(extension) == mime_types.end()) {
        return utils::kDefaultContentType();
    }
    return mime_types[extension];
}
