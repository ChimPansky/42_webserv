#include "AResponseProcessor.h"

#include "ErrorProcessor.h"

void AResponseProcessor::DelegateToErrProc(http::ResponseCode err_code)
{
    delegated_processor_.reset(new ErrorProcessor(dest_, response_rdy_cb_, err_code));
}
