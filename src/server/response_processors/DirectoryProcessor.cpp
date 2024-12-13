#include "response_processors/DirectoryProcessor.h"
#include "DirectoryProcessor.h"

#include <ResponseCodes.h>
#include <file_utils.h>
#include <unique_ptr.h>
#include <unistd.h>



DirectoryProcessor::DirectoryProcessor(const std::string& file_path,
                             utils::unique_ptr<http::IResponseCallback> response_rdy_cb)
    : AResponseProcessor(response_rdy_cb),
      err_response_processor_(utils::unique_ptr<GeneratedErrorResponseProcessor>(NULL))
{
    (void) file_path;

}
