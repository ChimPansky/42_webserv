#ifndef WS_HTTP_URI_H
#define WS_HTTP_URI_H

#include <string>
namespace http {

// Uri consists of: path, query, fragment
// e.g. /path/to/file?query1=1&query2=2#section1 
// to be decided later: do we need to add user info, host, port beforehand?
class Uri {
  public:
    Uri(const std::string& raw_uri);
    Uri(const std::string& path, const std::string& query, const std::string& fragment);
    Uri(const Uri& rhs);
    ~Uri() {}

    Uri& operator=(const Uri& rhs);
    bool operator==(const Uri& rhs) const;
    bool operator!=(const Uri& rhs) const;

    bool Good() const { return validity_state_ == URI_GOOD_BIT; };
    std::string ToStr() const;

    int status() const { return validity_state_; };

    const std::string& path() const { return path_; };
    const std::string& query() const { return query_; };
    const std::string& fragment() const { return fragment_; };

  private:
  // todo: dont forget to use...
    enum UriStatus {
        URI_GOOD_BIT = 0,
        URI_BAD_BIT = 1L << 0,
        URI_TOO_LONG_BIT = 1L << 3,
        URI_BAD_PATH_BIT = 1L << 7,
        URI_BAD_QUERY_BIT = 1L << 8,
        URI_BAD_FRAGMENT_BIT = 1L << 9,
        URI_FAIL_BIT = 1L << 16
    };

    enum ParseState {
        PS_PATH,
        PS_QUERY,
        PS_FRAGMENT,
        PS_END
    };

    int validity_state_;
    std::string path_; // "/", "/index.html", "/path/to/file"
    std::string query_; 
    std::string fragment_; // used to jump to specific location on website, e.g. "#section1", "#details", "#dashboard"

    void Validate_();
    void ParseRawUri_(const std::string& raw_uri);
    void ParsePath_(const std::string& raw_uri, size_t& raw_uri_pos, ParseState& state);
    void ParseQuery_(const std::string& raw_uri, size_t& raw_uri_pos, ParseState& state);
    void ParseFragment_(const std::string& raw_uri, size_t& raw_uri_pos, ParseState& state);

    // helpers:
    bool IsValidPathChar_(char c) const;
    bool IsValidQueryOrFragmentChar_(char c) const;

    bool IsValidPath_(const std::string& path) const;
    bool IsValidQuery_(const std::string& query) const;
    bool IsValidFragment_(const std::string& fragment) const;
};
} // namespace http

#endif // WS_HTTP_URI_H
