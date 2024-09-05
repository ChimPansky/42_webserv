#ifndef WS_LOCATIONBLOCK_H
#define WS_LOCATIONBLOCK_H

#include <map>
#include <string>
#include <vector>

typedef std::pair<std::string, std::string> setting;

class LocationBlock {
  private:
    typedef void (LocationBlock::*FunctionPointer)(const std::string&);
    typedef std::map<std::string, FunctionPointer>::const_iterator MethodsIt;
    std::map<std::string, FunctionPointer> InitLocationSettings();

  public:
    LocationBlock(const std::vector<setting>& settings);
    const std::string& route() const;
    const std::vector<std::string>& allowed_methods() const;
    const std::pair<int, std::string>& redirect() const;
    bool is_cgi() const;
    const std::vector<std::string>& cgi_paths() const;
    const std::vector<std::string>& cgi_extensions() const;
    const std::string& root_dir();
    const std::string& default_file() const;
    const std::string& dir_listing() const;
    static const std::vector<std::string> GetTokens();

    void InitRoute(const std::string& value);
    void InitAllowedMethods(const std::string& value);
    void InitRedirect(const std::string& value);
    void InitCgiPaths(const std::string& value);
    void InitCgiExtensions(const std::string& value);
    void InitRootDir(const std::string& value);
    void InitDefaultFile(const std::string& value);
    void InitDirListing(const std::string& value);

  private:
    std::string route_;
    std::vector<std::string> allowed_methods_;
    std::pair</* status code */ int, /* new route */ std::string> redirect_;
    bool is_cgi_;
    std::vector<std::string> cgi_paths_;
    std::vector<std::string> cgi_extensions_;
    std::string root_dir_;
    std::string default_file_;
    std::string dir_listing_;
};

#endif  // WS_LOCATIONBLOCK_H
