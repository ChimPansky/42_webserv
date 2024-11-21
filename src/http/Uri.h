#ifndef WS_HTTP_URI_H
#define WS_HTTP_URI_H

#include <string>
namespace http {

// Uri consists of: path, query, fragment
// e.g. /path/to/file?query1=1&query2=2#section1
// to be decided later: do we need to add user info, host, port beforehand?
// query and fragment can be defined by ? and # respectively. a defined query or fragment can
// be empty or non-empty: "/path?#section1", "/path?query1=1#"


class Uri {
  public:
    Uri(){};
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

    const std::string& scheme() const { return scheme_.second; };
    const std::string& user_info() const { return user_info_.second; };
    const std::string& host() const { return host_.second; };
    unsigned short port() const { return port_.second; };
    const std::string& path() const { return path_; };
    const std::string& query() const { return query_.second; };
    const std::string& fragment() const { return fragment_.second; };

  private:
    // todo: dont forget to use...
    enum UriStatus {
        URI_GOOD_BIT = 0,
        URI_BAD_BIT = 1L << 0,
        URI_TOO_LONG_BIT = 1L << 3,
        URI_BAD_PATH_BIT = 1L << 7,
        URI_BAD_QUERY_BIT = 1L << 8,
        URI_BAD_FRAGMENT_BIT = 1L << 9,
    };

    static const char* unreserved;
    static const char* gen_delims;
    static const char* sub_delims;
    int validity_state_;
    std::pair<bool /*defined*/, std::string /*value*/> scheme_;
    std::pair<bool /*defined*/, std::string /*value*/> user_info_;
    std::pair<bool /*defined*/, std::string /*value*/> host_;
    std::pair<bool /*defined*/, unsigned short /*value*/> port_;
    std::string path_;
    std::pair<bool /*defined*/, std::string /*value*/> query_;
    std::pair<bool /*defined*/, std::string /*value*/> fragment_;

    void Validate_();
    void ParsePath_(const std::string& raw_uri, size_t& raw_uri_pos);
    void ParseQuery_(const std::string& raw_uri, size_t& raw_uri_pos);
    void ParseFragment_(const std::string& raw_uri, size_t& raw_uri_pos);

    std::pair<bool /*valid*/, std::string> PercentDecode_(const std::string& str,
                                                          const char* ignore_set = NULL) const;
    std::pair<bool /*valid*/, std::string> Normalize_(const std::string& str) const;
    //Normalize should do the following 3:
    //RemoveDotSegments();
    //makeEncodedHexToUpper(); %2f -> %2F
    std::string CollapseChars_(const std::string& str, char c) const;

    // helpers:
    void RemoveLastSegment_(std::string& path) const;
    void MoveSegmentToOutput_(std::string& input, std::string& output) const;
    bool IsValidPathChar_(char c) const;
    bool IsValidQueryOrFragmentChar_(char c) const;

    bool IsValidPath_(const std::string& path) const;
    bool IsValidQuery_(const std::string& query) const;
    bool IsValidFragment_(const std::string& fragment) const;
};

std::ostream& operator<<(std::ostream& out, const Uri& uri);

}  // namespace http

#endif  // WS_HTTP_URI_H
