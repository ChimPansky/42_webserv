#include "response_processors/DirectoryProcessor.h"
#include "DirectoryProcessor.h"
#include "http.h"
#include "logger.h"

#include <ResponseCodes.h>
#include <file_utils.h>
#include <unique_ptr.h>
#include <unistd.h>



DirectoryProcessor::DirectoryProcessor(const std::string& file_path,
                             utils::unique_ptr<http::IResponseCallback> response_rdy_cb, const http::Request& rq, utils::shared_ptr<Location> loc)
    : AResponseProcessor(response_rdy_cb),
      err_response_processor_(utils::unique_ptr<GeneratedErrorResponseProcessor>(NULL)), rq_(rq)
{
    if (rq_.method == http::HTTP_GET) {
        if (loc->dir_listing()) {
            LOG(DEBUG) << "Listing directory";
            ListDirectory_(file_path);
        } else if (loc->default_files().size() > 0) {
            LOG(DEBUG) << "Serve default file: " << loc->default_files()[0];
        } else {
            LOG(DEBUG) << "Directory listing is disabled"; // 401 or 403 or 404
            err_response_processor_ = utils::unique_ptr<GeneratedErrorResponseProcessor>(
                new GeneratedErrorResponseProcessor(response_rdy_cb_, http::HTTP_FORBIDDEN));
        }
    } else if (rq_.method == http::HTTP_POST)  {
        LOG(DEBUG) << "POST method logic for directory...";
    } else if (rq_.method == http::HTTP_DELETE) {
        LOG(DEBUG) << "DELETE method logic for directory...";
    } else {
        throw std::logic_error("Unknown Method for DirectoryProcessor");
    }
}

void DirectoryProcessor::ListDirectory_(const std::string& path) {
    LOG(DEBUG) << "Listing directory";
    std::map<std::string, std::string> hdrs;
    std::string body_str = "<html>\n"
                           "<head>\n"
                           "<title>Index of " + path + "</title>\n"
                           "</head>\n"
                           "<body>\n"
                           "<h1>Index of " + path + "</h1>\n"
                           "<ul>\n";
    std::vector<char> body;
    std::copy(body_str.begin(), body_str.end(), std::back_inserter(body));
    hdrs["Content-Type"] = "text/html";
    hdrs["Content-Length"] = utils::NumericToString(body.size());
    response_rdy_cb_->Call(utils::unique_ptr<http::Response> (new http::Response(http::HTTP_OK, http::HTTP_1_1, hdrs, body)));
}