#ifndef WS_HTTP_RQ_TARGET_H
#define WS_HTTP_RQ_TARGET_H

#include <cstddef>
#include <string>
namespace http {

/* RqTarget class:
 Request Target is the string that comes after the method and before the version in the request line. Example: "GET /path/to/file?query1=1&query2=2 HTTP/1.1"
 We need to handle 2 kinds of targets:
    1. Absolute path: /path[?query] (slash followed by zero or more path segments and an optional query). example: /path/to/file?query1=1&query2=2
    2. Absolute URI: scheme://[user_info@]host[:port]/path[?query][#fragment] (scheme followed by host followed by optional port followed by absolute path. example: http://www.example.com/path/to/file?query1=1&query2=2

    - Mostly we handle RqTargets in first form, but according to RFC we also need to be able to accept Requests with Targets in second form (Requests to Proxy servers).
    - The host and port information in the second form will then override the value of the host header field and will be used to choose a Server from the ServerCluster.

    in order to make the target comparable to other targets, we need to normalize it. Normalization includes:
    - Store scheme and host in lower-case since they are case-insensitive.
    - Percent-decode the path and query components. Only decode unreserved characters, leave reserved characters (chars that are used as delimiters like %2F for /, %3F for ?,...) as is.
    - Remove dot-segments from the path component.
    - Make the encoded hex characters in the path and query components uppercase.
    - Collapse sequences of slashes in the path-component.

*/

class RqTarget {
  public:
    RqTarget(){};
    RqTarget(const std::string& raw_uri);
    RqTarget(const std::string& scheme, const std::string& user_info, const std::string& host, const std::string& port, const std::string& path, const std::string query, const std::string& fragment);
    RqTarget(const RqTarget& rhs);
    ~RqTarget() {}

    RqTarget& operator=(const RqTarget& rhs);
    bool operator==(const RqTarget& rhs) const;
    bool operator!=(const RqTarget& rhs) const;

    bool Good() const { return validity_state_ == TARGET_GOOD; };
    std::string ToStr() const;
    std::string GetDebugString() const;

    int status() const { return validity_state_; };

    const std::string& scheme() const { return scheme_.second; };
    const std::string& user_info() const { return user_info_.second; };
    const std::string& host() const { return host_.second; };
    const std::string& port() const { return port_.second; };
    const std::string& path() const { return path_.second; };
    const std::string& query() const { return query_.second; };
    const std::string& fragment() const { return fragment_.second; };

    typedef std::pair<bool /*component_defined*/, std::string /*value*/> UriComponent;

  private:
    // todo: dont forget to use...
    enum RqTargetStatus {
        TARGET_GOOD = 0,
        TARGET_BAD = 1L << 0,
        TARGET_TOO_LONG = 1L << 3,
        TARGET_BAD_SCHEME = 1L << 4,
        TARGET_HAS_USER_INFO = 1L << 5,
        TARGET_BAD_HOST = 1L << 6,
        TARGET_BAD_PORT = 1L << 7,
        TARGET_BAD_PATH = 1L << 8,
        TARGET_BAD_QUERY = 1L << 9,
        TARGET_HAS_FRAGMENT = 1L << 10,
    };

    static const char* unreserved;
    static const char* gen_delims;
    static const char* sub_delims;
    int validity_state_;
    UriComponent scheme_;
    UriComponent user_info_;
    UriComponent host_;
    UriComponent port_;
    UriComponent path_;
    UriComponent query_;
    UriComponent fragment_;

    void Validate_();
    bool ExtractComponent();
    void ParseScheme_(const std::string& raw_uri, size_t& raw_uri_pos);
    void ParseUserInfo_(const std::string& raw_uri, size_t& raw_uri_pos);
    void ParseHost_(const std::string& raw_uri, size_t& raw_uri_pos);
    void ParsePort_(const std::string& raw_uri, size_t& raw_uri_pos);
    void ParsePath_(const std::string& raw_uri, size_t& raw_uri_pos);
    void ParseQuery_(const std::string& raw_uri, size_t& raw_uri_pos);
    void ParseFragment_(const std::string& raw_uri, size_t& raw_uri_pos);

    std::pair<bool /*valid*/, std::string> PercentDecode_(const std::string& str,
                                                          const char* ignore_set = NULL) const;
    std::pair<bool /*valid*/, std::string> RemoveDotSegments_(const std::string& str) const;
    //Normalize should do the following 3:
    //RemoveDotSegments();
    //makeEncodedHexToUpper(); %2f -> %2F
    std::string CollapseChars_(const std::string& str, char c) const;

    // helpers:
    void RemoveLastSegment_(std::string& path) const;
    void MoveSegmentToOutput_(std::string& input, std::string& output) const;
    bool IsValidPathChar_(char c) const;
    bool IsValidQueryOrFragmentChar_(char c) const;

    void ValidateScheme_();
    bool IsValidPath_(const std::string& path) const;
    bool IsValidQuery_(const std::string& query) const;
    bool IsValidFragment_(const std::string& fragment) const;
};

std::ostream& operator<<(std::ostream& out, const RqTarget& RqTarget);

}  // namespace http

#endif  // WS_HTTP_URI_H
