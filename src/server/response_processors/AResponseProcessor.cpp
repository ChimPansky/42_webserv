#include "AResponseProcessor.h"

#include "ErrorProcessor.h"

void AResponseProcessor::DelegateToErrProc(http::ResponseCode err_code)
{
    delegated_processor_.reset(new ErrorProcessor(server_, response_rdy_cb_, err_code));
}
