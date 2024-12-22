#include "DirectoryProcessor.h"

#include <sstream>

#include "ErrorProcessor.h"
#include "ResponseCodes.h"
#include "http.h"
#include "logger.h"
#include "response_processors/AResponseProcessor.h"
#include "response_processors/RedirectProcessor.h"


const std::string DirectoryProcessor::kAutoIndexStyle()
{
    return "body { font-family: 'Courier New', Courier, monospace; background-color: "
           "#121212;color: #e0e0e0;margin: 0; padding: 0; }"
           "h1 { padding: 20px; background-color: #1e1e1e; margin: 0; border-bottom: 1px solid "
           "#444; }"
           "table { width: 100%; border-collapse: collapse; margin: 20px 0; }"
           "th, td { padding: 10px; text-align: left; border: 1px solid #444; }"
           "th { background-color: #1e1e1e;    color: #e0e0e0; text-transform: uppercase; "
           "font-size: 14px; }"
           "tr:nth-child(odd) { background-color: #1a1a1a; }"
           "tr:nth-child(even) { background-color: #2a2a2a; }"
           "a { color: #80c0ff; text-decoration: none; }"
           "a:hover { text-decoration: underline; }";
}

DirectoryProcessor::DirectoryProcessor(const Server& server,
                                       utils::unique_ptr<http::IResponseCallback> response_rdy_cb,
                                       const http::Request& rq, const std::string& file_path)
    : AResponseProcessor(server, response_rdy_cb)
{
    if (*file_path.rbegin() != '/') {
        LOG(DEBUG) << "Path is a directory but does not end with / -> Redirect";
        http::RqTarget redirected_target = rq.rqTarget;
        redirected_target.AddTrailingSlashToPath();
        delegated_processor_.reset(new RedirectProcessor(
            server_, response_rdy_cb_, http::HTTP_MOVED_PERMANENTLY, redirected_target.ToStr()));
        return;
    }
    if (rq.method != http::HTTP_GET) {
        delegated_processor_ = utils::unique_ptr<AResponseProcessor>(
            new ErrorProcessor(server, response_rdy_cb_, http::HTTP_METHOD_NOT_ALLOWED));
        return;
    }
    if (!ListDirectory_(file_path)) {
        delegated_processor_ = utils::unique_ptr<AResponseProcessor>(
            new ErrorProcessor(server, response_rdy_cb_, http::HTTP_INTERNAL_SERVER_ERROR));
        return;
    }
}

bool DirectoryProcessor::ListDirectory_(const std::string& path)
{
    std::pair<bool, std::vector<utils::DirEntry> > dir_entries = utils::GetDirEntries(path.c_str());
    if (!dir_entries.first) {
        return false;
    }
    std::map<std::string, std::string> hdrs;
    std::sort(dir_entries.second.begin(), dir_entries.second.end());
    std::ostringstream body_stream;
    GenerateAutoIndexPage_(body_stream, path, dir_entries.second);
    std::string body_string = body_stream.str();
    std::vector<char> body(body_string.begin(), body_string.end());
    hdrs["Content-Type"] = "text/html";
    hdrs["Content-Length"] = utils::NumericToString(body.size());
    response_rdy_cb_->Call(utils::unique_ptr<http::Response>(
        new http::Response(http::HTTP_OK, http::HTTP_1_1, hdrs, body)));
    return true;
}

void DirectoryProcessor::GenerateAutoIndexPage_(std::ostringstream& body, const std::string& path,
                                                const std::vector<utils::DirEntry>& entries)
{
    body << "<html>\n<head>\n<title>Index of " << path << "</title>\n</head>\n"
         << "<style>\n"
         << kAutoIndexStyle()
         << "\n</style>\n"
            "<base href=\""
         << "\">"
            "<body>\n<h1>Index of "
         << path
         << "</h1>\n"
            "<table border=\"0\">\n"
            "<thead>\n"
            "<tr>\n"
            "<th>Name</th>\n"
            "<th>Last Modified</th>\n"
            "<th>Size (Bytes)</th>\n"
            "</tr>\n"
            "</thead>\n"
            "<tbody>\n";

    for (size_t i = 0; i < entries.size(); i++) {
        const utils::DirEntry& entry = entries[i];
        std::string time_str = utils::GetFormatedTime(entry.last_modified());
        if (entry.name() == "./") {
            continue;
        }
        body << "<tr><td><a href=\"" << entry.name() << "\"";
        if (entry.type() == utils::DE_FILE) {
            body << " target=\"_blank\"";
        }
        body << ">";
        if (entry.name() == "../") {
            body << "&#x21B0; Parent Directory";
        } else {
            body << entry.name();
        }
        body << "</a></td>\n";
        body << "<td>" << time_str
             << "</td>\n"
                "<td>";
        if (entry.type() == utils::DE_FILE) {
            body << entry.size();
        };
        body << "</td>\n"
                "</tr>\n";
    }
    body << "</tbody>\n</table>\n</body>\n</html>\n";
}
