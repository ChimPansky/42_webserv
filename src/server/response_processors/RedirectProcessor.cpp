#include "RedirectProcessor.h"

#include "../Server.h"
#include "ResponseCodes.h"

RedirectProcessor::RedirectProcessor(RequestDestination dest,
                                     utils::unique_ptr<http::IResponseCallback> response_rdy_cb,
                                     http::ResponseCode code, const std::string& redirect_uri)
    : AResponseProcessor(dest, response_rdy_cb)
{
    if (code < 300) {
        DelegateToErrProc(http::HTTP_INTERNAL_SERVER_ERROR);
        return;
    } else if (code >= 400) {
        DelegateToErrProc(code);
        return;
    }
    std::map<std::string, std::string> hdrs;
    hdrs["Location"] = redirect_uri;
    response_rdy_cb_->Call(utils::unique_ptr<http::Response>(
        new http::Response(code, http::HTTP_1_1, hdrs, std::vector<char>())));
}
