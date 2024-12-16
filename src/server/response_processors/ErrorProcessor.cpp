#include "ErrorProcessor.h"

#include "../Server.h"
#include "logger.h"

ErrorProcessor::ErrorProcessor(const Server& server,
                               utils::unique_ptr<http::IResponseCallback> response_rdy_cb,
                               http::ResponseCode code)
    : AResponseProcessor(server, response_rdy_cb)
{
    LOG(DEBUG) << "ErrorProcessor: code: " << code;
    typedef std::map<int, std::string>::const_iterator ErrPageIt;
    ErrPageIt err_page_it = server.error_pages().find(static_cast<int>(code));
    if (err_page_it == server.error_pages().end()) {
        delegated_processor_.reset(new GeneratedErrorProcessor(server_, response_rdy_cb_, code));
        return;
    }
    const std::string& file_path = err_page_it->second;
    if (!utils::DoesPathExist(file_path.c_str()) || utils::IsDirectory(file_path.c_str())) {
        LOG(ERROR) << "Error (" << code << ") page file not found by path: " << file_path;
        delegated_processor_.reset(new GeneratedErrorProcessor(server_, response_rdy_cb_, code));
        return;
    }
    std::ifstream file(file_path.c_str(), std::ios::binary);
    if (!file.is_open()) {
        LOG(DEBUG) << "Error (" << code << ") page file cannot be opened: " << file_path;
        delegated_processor_.reset(new GeneratedErrorProcessor(server_, response_rdy_cb_, code));
        return;
    }
    std::vector<char> body =
        std::vector<char>(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
    std::map<std::string, std::string> hdrs;
    hdrs["Content-Type"] = "text/html";
    hdrs["Connection"] = "Closed";
    hdrs["Content-Length"] = utils::NumericToString(body.size());
    response_rdy_cb_->Call(utils::unique_ptr<http::Response>(
        new http::Response(http::HTTP_OK, http::HTTP_1_1, hdrs, body)));
}

ErrorProcessor::GeneratedErrorProcessor::GeneratedErrorProcessor(
    const Server& server, utils::unique_ptr<http::IResponseCallback> response_rdy_cb,
    http::ResponseCode code)
    : AResponseProcessor(server, response_rdy_cb)
{
    std::string body_str = GenerateErrorPage_(code);
    std::vector<char> body;
    body.reserve(body_str.size());
    std::copy(body_str.begin(), body_str.end(), std::back_inserter(body));
    std::map<std::string, std::string> hdrs;
    hdrs["Content-Type"] = "text/html";
    hdrs["Connection"] = "Closed";
    hdrs["Content-Length"] = utils::NumericToString(body.size());
    response_rdy_cb_->Call(
        utils::unique_ptr<http::Response>(new http::Response(code, http::HTTP_1_1, hdrs, body)));
}

std::string ErrorProcessor::GeneratedErrorProcessor::GenerateErrorPage_(http::ResponseCode code)
{
    LOG(DEBUG) << "Generating error page for code: " << code;
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
