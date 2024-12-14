#include "response_processors/DirectoryProcessor.h"

#include <ResponseCodes.h>
#include <dirent.h>
#include <file_utils.h>
#include <unique_ptr.h>
#include <unistd.h>

#include <utility>

#include "DirectoryProcessor.h"
#include "http.h"
#include "logger.h"


DirectoryProcessor::DirectoryProcessor(utils::unique_ptr<http::IResponseCallback> response_rdy_cb,
                                       const std::string& file_path, const http::Request& rq,
                                       utils::shared_ptr<Location> loc)
    : AResponseProcessor(response_rdy_cb),
      err_response_processor_(utils::unique_ptr<GeneratedErrorResponseProcessor>(NULL)), rq_(rq)
{
    if (rq_.method == http::HTTP_GET) {
        if (loc->dir_listing()) {
            LOG(DEBUG) << "Listing directory";
            if (!ListDirectory_(file_path)) {
                err_response_processor_ = utils::unique_ptr<GeneratedErrorResponseProcessor>(
                    new GeneratedErrorResponseProcessor(response_rdy_cb_,
                                                        http::HTTP_INTERNAL_SERVER_ERROR));
            }
        } else if (loc->default_files().size() > 0) {
            LOG(DEBUG) << "Serve default file: " << loc->default_files()[0];
        } else {
            LOG(DEBUG) << "Directory listing is disabled";  // 401 or 403 or 404
            err_response_processor_ = utils::unique_ptr<GeneratedErrorResponseProcessor>(
                new GeneratedErrorResponseProcessor(response_rdy_cb_, http::HTTP_FORBIDDEN));
        }
    } else if (rq_.method == http::HTTP_POST) {
        LOG(DEBUG) << "POST method logic for directory...";
    } else if (rq_.method == http::HTTP_DELETE) {
        LOG(DEBUG) << "DELETE method logic for directory...";
    } else {
        throw std::logic_error("Unknown Method for DirectoryProcessor");
    }
}

bool DirectoryProcessor::ListDirectory_(const std::string& path)
{
    LOG(DEBUG) << "ListDirectory_";
    LOG(DEBUG) << "Path: " << path;
    std::pair<bool, std::vector<DirEntry> > dir_entries = GetDirEntries_(path.c_str());
    if (!dir_entries.first) {
        return false;
    }
    std::map<std::string, std::string> hdrs;
    std::ostringstream body_stream;
    body_stream << "<html>\n"
                   "<head>\n"
                   "<title>Index of "
                << path
                << "</title>\n"
                   "</head>\n"
                   "<body>\n"
                   "<h1>Index of "
                << path
                << "</h1>\n"
                   "<ul>\n";
    for (size_t i = 0; i < dir_entries.second.size(); i++) {
        body_stream << "<li><a href=\"" << path << "/" << dir_entries.second[i].name() << "\">"
                    << dir_entries.second[i].name() << "</a></li>\n";
    }
    body_stream << "</ul>\n</body>\n</html>\n";

    LOG(DEBUG) << "Body: " << body_stream.str();
    std::string body_str = body_stream.str();
    std::vector<char> body(body_str.begin(), body_str.end());
    hdrs["Content-Type"] = "text/html";
    hdrs["Content-Length"] = utils::NumericToString(body.size());
    // todo: instead of creating the whole response at once, stream the body to the client with
    // chunks...
    response_rdy_cb_->Call(utils::unique_ptr<http::Response>(
        new http::Response(http::HTTP_OK, http::HTTP_1_1, hdrs, body)));
    return true;
}


std::pair<bool /*success*/, std::vector<DirectoryProcessor::DirEntry> /*dir_entries*/>
DirectoryProcessor::GetDirEntries_(const char* directory)
{
    DIR* dir = opendir(directory);
    std::vector<DirectoryProcessor::DirEntry> entries;
    if (dir == NULL) {
        LOG(ERROR) << "Unable to read directory";
        return std::make_pair(false, entries);
    }
    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        LOG(DEBUG) << "Entry: " << entry->d_name;
        entries.push_back(
            DirectoryProcessor::DirEntry(entry->d_name, std::string("<path_fo_file>"), DE_FILE));
    }
    closedir(dir);
    return std::make_pair(true, entries);
}
