#include "RedirectProcessor.h"

#include "../Server.h"

RedirectProcessor::RedirectProcessor(const Server& server,
                                     utils::unique_ptr<http::IResponseCallback> response_rdy_cb,
                                     http::ResponseCode code, const std::string& redirect_uri)
    : AResponseProcessor(server, response_rdy_cb)
{
    std::map<std::string, std::string> hdrs;
    hdrs["Connection"] = "Close";
    hdrs["Location"] = redirect_uri;
    response_rdy_cb_->Call(utils::unique_ptr<http::Response>(
        new http::Response(code, http::HTTP_1_1, hdrs, std::vector<char>())));
}
