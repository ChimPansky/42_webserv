#ifndef WS_SERVERBLOCK_H
#define WS_SERVERBLOCK_H

#include <netinet/in.h>
#include <utils/unique_ptr.h>

#include <map>
#include <string>

#include "LocationBlock.h"

class ServerBlock {
  private:
    typedef std::pair<std::string, std::string> S;
    typedef void (ServerBlock::*FunctionPointer)(const std::string&);
    typedef std::map<std::string, FunctionPointer>::const_iterator MethodsIt;
    std::map<std::string, FunctionPointer> InitServerSettings();
    void InitAccessLog(const std::string& value);
    void InitErrorLogPath(const std::string& value);
    void InitListeners(const std::string& value);
    void InitRootDir(const std::string& value);
    void InitDefaultFile(const std::string& value);
    void InitDirListing(const std::string& value);
    void InitServerNames(const std::string& value);
    void InitLocations(const std::string& value);

  public:
    ServerBlock(std::vector<S> settings);
    const std::string& access_log_path() const;
    const std::string& access_log_level() const;
    const std::string& error_log_path() const;
    const std::vector<std::pair<in_addr_t, in_port_t> >& listeners() const;
    const std::string& root_dir();
    const std::string& default_file();
    const std::string& dir_listing();
    const std::vector<std::string>& server_names();
    const std::map<std::string, LocationBlock>& locations();
    static std::vector<S> ExtractBlock(std::vector<S>::iterator& it);

  private:
    std::string access_log_path_;
    std::string access_log_level_;
    std::string error_log_path_;
    std::vector<std::pair<in_addr_t, in_port_t> > listeners_;
    std::string root_dir_;
    std::string default_file_;
    std::string dir_listing_;
    std::vector<std::string> server_names_;
    std::map<std::string, LocationBlock> locations_;
};

#endif  // WS_SERVERBLOCK_H
