#include "response_processors/DirectoryProcessor.h"

#include <ResponseCodes.h>
#include <dirent.h>
#include <file_utils.h>
#include <str_utils.h>
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

bool DirectoryProcessor::ListDirectory_(const std::string& path, const std::string& location_root_dir)
{
    LOG(DEBUG) << "ListDirectory_";
    LOG(DEBUG) << "Path: " << path;
    std::pair<bool, std::vector<DirEntry> > dir_entries = GetDirEntries_(path.c_str());
    if (!dir_entries.first) {
        return false;
    }
    std::map<std::string, std::string> hdrs;
    std::ostringstream body_stream;

    LOG(DEBUG) << "path: " << path;
    std::string entry_rel_folder = RemoveRootFromPath(path, location_root_dir);
    if (entry_rel_folder.empty() || entry_rel_folder[entry_rel_folder.size() - 1] != '/') {
        entry_rel_folder += "/";
    }
    LOG(DEBUG) << "Location root dir: " << location_root_dir;
    LOG(DEBUG) << "Entry relative folder: " << entry_rel_folder;
    body_stream << "<html>\n<head>\n<title>Index of " << path << "</title>\n</head>\n"
                   "<body>\n<h1>Index of " << path << "</h1>\n"
                   "<ul>\n";
    for (size_t i = 0; i < dir_entries.second.size(); i++) {
        body_stream << "<li><a href=\"" << entry_rel_folder << dir_entries.second[i].name() << "\"";
        if (dir_entries.second[i].type() == DE_FILE) {  // open files in new tab
            body_stream << " target=\"_blank\"";
        }
        body_stream << ">" << dir_entries.second[i].name() << "</a><a>\t" << path << dir_entries.second[i].name() << "</a></li>\n";
    }
    body_stream << "</ul>\n"
                    "</body>\n</html>\n";

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
        LOG(DEBUG) << "Dirent contents: " << entry->d_name << " | " << entry->d_type << " | " << entry->d_reclen << " | " << entry->d_off << " | " << entry->d_ino;
        entries.push_back(
            DirectoryProcessor::DirEntry(entry->d_name, std::string("<path_fo_file>"), (entry->d_type == DT_DIR ? DE_DIR :  DE_FILE)));
            // use stat() to get more info about the file
    }
    closedir(dir);
    return std::make_pair(true, entries);
}


//     void printTimestamps(const char* path) {
//     struct stat fileStat;

//     if (stat(path, &fileStat) == 0) {
//         // Last modified time
//         std::cout << "Last Modified: " << std::ctime(&fileStat.st_mtime);

//         // Creation time (Not supported on all POSIX systems, use `st_ctime`)
//         #ifdef __APPLE__
//         std::cout << "Created: " << std::ctime(&fileStat.st_birthtime);
//         #else
//         std::cout << "Created (ctime): " << std::ctime(&fileStat.st_ctime);
//         #endif
//     } else {
//         perror("stat");
//     }
// }

std::string DirectoryProcessor::RemoveRootFromPath(const std::string& path, const std::string& root) {
    if (root.empty()) {
        return path;
    }
    std::string prefix = root[0] == '/' ? root.substr(1) : root;
    if (path.compare(0, prefix.size(), prefix) == 0) {
        return path.substr(prefix.size());
    }
    return path;
}