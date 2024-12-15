#include "DirectoryProcessor.h"

#include <dirent.h>

#include "http.h"
#include "logger.h"

const std::string DirectoryProcessor::GetDirStyle_()
{
    return "<style>\n"
           "body {\n"
           "    font-family: 'Courier New', Courier, monospace;\n"
           "    background-color: #121212;\n"
           "    color: #e0e0e0;\n"
           "    margin: 0;\n"
           "    padding: 0;\n"
           "}\n"
           "h1 {\n"
           "    padding: 20px;\n"
           "    background-color: #1e1e1e;\n"
           "    margin: 0;\n"
           "    border-bottom: 1px solid #444;\n"
           "}\n"
           "table {\n"
           "    width: 100%;\n"
           "    border-collapse: collapse;\n"
           "    margin: 20px 0;\n"
           "}\n"
           "th, td {\n"
           "    padding: 10px;\n"
           "    text-align: left;\n"
           "    border: 1px solid #444;\n"
           "}\n"
           "th {\n"
           "    background-color: #1e1e1e;\n"
           "    color: #e0e0e0;\n"
           "    text-transform: uppercase;\n"
           "    font-size: 14px;\n"
           "}\n"
           "tr:nth-child(odd) {\n"
           "    background-color: #1a1a1a;\n"
           "}\n"
           "tr:nth-child(even) {\n"
           "    background-color: #2a2a2a;\n"
           "}\n"
           "a {\n"
           "    color: #80c0ff;\n"
           "    text-decoration: none;\n"
           "}\n"
           "a:hover {\n"
           "    text-decoration: underline;\n"
           "}\n"
           "</style>\n";
}

DirectoryProcessor::DirectoryProcessor(utils::unique_ptr<http::IResponseCallback> response_rdy_cb,
                                       const std::string& file_path, const http::Request& rq,
                                       utils::shared_ptr<Location> loc)
    : AResponseProcessor(response_rdy_cb),
      err_response_processor_(utils::unique_ptr<GeneratedErrorResponseProcessor>(NULL)), rq_(rq)
{
    // LOG(DEBUG) << "DirectoryProcessor";
    // LOG(DEBUG) << "Location: " << loc->GetDebugString();
    if (rq_.method == http::HTTP_GET) {
        if (loc->dir_listing()) {
            LOG(DEBUG) << "Listing directory";
            if (!ListDirectory_(file_path, loc->root_dir())) {
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

bool DirectoryProcessor::ListDirectory_(const std::string& path,
                                        const std::string& location_root_dir)
{
    std::pair<bool, std::vector<DirEntry> > dir_entries = GetDirEntries_(path.c_str());
    if (!dir_entries.first) {
        return false;
    }
    std::map<std::string, std::string> hdrs;
    std::ostringstream body_stream;

    std::string entry_rel_folder = RemoveRootFromPath(path, location_root_dir);
    if (entry_rel_folder.empty() || entry_rel_folder[entry_rel_folder.size() - 1] != '/') {
        entry_rel_folder += "/";
    }
    body_stream << "<html>\n<head>\n<title>Index of " << path << "</title>\n</head>\n"
                << GetDirStyle_() << "<body>\n<h1>Index of " << path
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

    std::sort(dir_entries.second.begin(), dir_entries.second.end(), CompareDirEntriesByName);
    for (size_t i = 0; i < dir_entries.second.size(); i++) {
        const DirEntry& entry = dir_entries.second[i];
        std::string time_str = utils::GetFormatedTime(entry.last_modified());

        body_stream << "<tr>\n"
                       "<td><a href=\""
                    << entry_rel_folder << entry.name() << "\"";
        if (entry.type() == DE_FILE) {  // open files in new tab
            body_stream << " target=\"_blank\"";
        }
        body_stream << ">" << entry.name() << "</a></td>\n";
        body_stream << "<td>" << time_str
                    << "</td>\n"
                       "<td>"
                    << entry.size()
                    << "</td>\n"
                       "</tr>\n";
    }
    body_stream << "</tbody>\n</table>\n</body>\n</html>\n";

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
        LOG(ERROR) << "Unable to read directory: " << directory;
        return std::make_pair(false, entries);
    }

    struct dirent* entry;
    while ((entry = readdir(dir)) != NULL) {
        std::string filePath = std::string(directory) + "/" + entry->d_name;
        struct stat fileStat;
        if (stat(filePath.c_str(), &fileStat) == 0) {
            std::time_t lastModified = fileStat.st_mtime;
            size_t size = static_cast<size_t>(fileStat.st_size);

            DirectoryProcessor::DirEntryType type = (entry->d_type == DT_DIR ? DE_DIR : DE_FILE);

            entries.push_back(DirEntry(entry->d_name, filePath, type, lastModified, size));
        } else {
            LOG(ERROR) << "Unable to read directory stats for file: " << filePath;
        }
    }
    closedir(dir);
    return std::make_pair(true, entries);
}

std::string DirectoryProcessor::RemoveRootFromPath(const std::string& path, const std::string& root)
{
    if (root.empty()) {
        return path;
    }
    std::string prefix = root[0] == '/' ? root.substr(1) : root;
    if (path.compare(0, prefix.size(), prefix) == 0) {
        return path.substr(prefix.size());
    }
    return path;
}

bool CompareDirEntriesByName(const DirectoryProcessor::DirEntry& a,
                             const DirectoryProcessor::DirEntry& b)
{
    return a.name() < b.name();
}
