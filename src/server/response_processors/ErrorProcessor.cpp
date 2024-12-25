#include "ErrorProcessor.h"

#include <logger.h>

#include "../Server.h"

ErrorProcessor::ErrorProcessor(RequestDestination dest,
                               utils::unique_ptr<http::IResponseCallback> response_rdy_cb,
                               http::ResponseCode code)
    : AResponseProcessor(dest, response_rdy_cb)
{
    typedef std::map<int, std::string>::const_iterator ErrPageIt;
    ErrPageIt err_page_it = dest.server->error_pages().find(static_cast<int>(code));
    if (err_page_it == dest.server->error_pages().end()) {
        delegated_processor_.reset(new GeneratedErrorProcessor(dest, response_rdy_cb_, code));
        return;
    }
    const std::string& file_path = err_page_it->second;
    if (!utils::DoesPathExist(file_path.c_str()) || utils::IsDirectory(file_path.c_str())) {
        LOG(ERROR) << "Error (" << code << ") page file not found by path: " << file_path;
        delegated_processor_.reset(new GeneratedErrorProcessor(dest, response_rdy_cb_, code));
        return;
    }
    if (!utils::IsRegularFile(file_path.c_str()) || !utils::IsReadable(file_path.c_str())) {
        LOG(DEBUG) << "Error (" << code << ") page file cannot be opened: " << file_path;
        delegated_processor_.reset(new GeneratedErrorProcessor(dest, response_rdy_cb_, code));
        return;
    }
    std::map<std::string, std::string> hdrs;
    hdrs["Content-Type"] = "text/html";
    hdrs["Connection"] = "Close";
    hdrs["Content-Length"] = utils::NumericToString(utils::GetFileSize(file_path.c_str()) - 1);
    response_rdy_cb_->Call(utils::unique_ptr<http::Response>(
        new http::Response(code, http::HTTP_1_1, hdrs, file_path)));
}

ErrorProcessor::GeneratedErrorProcessor::GeneratedErrorProcessor(
    RequestDestination dest, utils::unique_ptr<http::IResponseCallback> response_rdy_cb,
    http::ResponseCode code)
    : AResponseProcessor(dest, response_rdy_cb)
{
    std::string body_str = GenerateErrorPage_(code);
    std::vector<char> body;
    body.reserve(body_str.size());
    std::copy(body_str.begin(), body_str.end(), std::back_inserter(body));
    std::map<std::string, std::string> hdrs;
    hdrs["Content-Type"] = "text/html";
    hdrs["Connection"] = "Close";
    hdrs["Content-Length"] = utils::NumericToString(body.size());
    response_rdy_cb_->Call(
        utils::unique_ptr<http::Response>(new http::Response(code, http::HTTP_1_1, hdrs, body)));
}

std::string ErrorProcessor::GeneratedErrorProcessor::GenerateErrorPage_(http::ResponseCode code)
{
    std::stringstream ss;
    ss << "<!DOCTYPE html>\n"
       << "<html lang=\"en\">"
       << "<head>"
       << "<meta charset=\"UTF-8\">"
       << "<title>" << code << " " << http::GetResponseCodeDescr(code) << "</title>"
       << "</head>"
       << "<body style=\"font-family: Arial, sans-serif; text-align: center; color: black; "
          "background-color: white; padding: 20px;\">"
       << "<h1 style=\"font-size: 48px; margin: 0;\">" << code << "</h1>"
       << "<p style=\"font-size: 16px; margin: 10px 0;\">" << http::GetResponseCodeDescr(code)
       << "</p>"
       << "</body>"
       << "</html>";
    return ss.str();
}
