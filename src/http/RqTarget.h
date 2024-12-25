#ifndef WS_HTTP_RQ_TARGET_H
#define WS_HTTP_RQ_TARGET_H

#include <http.h>
#include <numeric_utils.h>

#include <string>

namespace http {

/* RqTarget class:
 Request Target is the string that comes after the method and before the version in the request
 line. Example: "GET /path/to/file?query1=1&query2=2 HTTP/1.1" We need to handle 2 kinds of targets:
    1. Absolute path: /path[?query] (slash followed by zero or more path segments and an optional
 query). example: /path/to/file?query1=1&query2=2
    2. Absolute URI: scheme://host[:port]/path[?query] (scheme followed by host followed by optional
 port followed by absolute path followed by optional query. example:
 http://www.example.com/path/to/file?query1=1&query2=2

    - Mostly we handle RqTargets in first form, but according to RFC we also need to be able to
 accept Requests with Targets in second form (Requests to Proxy servers).
    - The host and port information in the second form will then override the value of the host
 header field and will be used to choose a Server from the ServerCluster.

    in order to make the target comparable to other targets, we need to normalize it. Normalization
 includes:
    - Store scheme and host in lower-case since they are case-insensitive.
    - Percent-decode the host, path and query components. Only decode unreserved characters, leave
 reserved characters (chars that are used as delimiters like %2F for /, %3F for ?,...) as is (might
 need to change this later - host and query may have different decoding rules).
    - Make the encoded hex characters in the path and query components uppercase.
    - Remove dot-segments from the path component.
    - Collapse sequences of slashes in the path-component.

*/

class RqTarget {
  public:
    enum RqTargetStatus {
        RQ_TARGET_GOOD = 0,
        RQ_TARGET_TOO_LONG = 1L << 3,
        RQ_TARGET_BAD_SCHEME = 1L << 4,
        RQ_TARGET_HAS_USER_INFO = 1L << 5,
        RQ_TARGET_BAD_HOST = 1L << 6,
        RQ_TARGET_BAD_PORT = 1L << 7,
        RQ_TARGET_BAD_PATH = 1L << 8,
        RQ_TARGET_BAD_QUERY = 1L << 9,
        RQ_TARGET_HAS_FRAGMENT = 1L << 10,
    };
    RqTarget();
    RqTarget(const std::string& raw_uri);
    RqTarget(const std::string& scheme, const std::string& user_info, const std::string& host,
             const std::string& port, const std::string& path, const std::string query,
             const std::string& fragment);
    RqTarget(const RqTarget& rhs);
    ~RqTarget() {}
    RqTarget& operator=(const RqTarget& rhs);
    bool operator==(const RqTarget& rhs) const;
    bool operator!=(const RqTarget& rhs) const;

    int validity_state() const { return validity_state_; };
    bool Good() const { return validity_state_ == RQ_TARGET_GOOD; };

    bool HasScheme() const { return scheme_.ok(); };
    bool HasUserInfo() const { return user_info_.ok(); };
    bool HasHost() const { return host_.ok(); };
    bool HasPort() const { return port_.ok(); };
    bool HasPath() const { return path_.ok(); };
    bool HasQuery() const { return query_.ok(); };
    bool HasFragment() const { return fragment_.ok(); };
    std::string ToStr() const;
    void AddTrailingSlashToPath();
    std::string GetDebugString() const;

    const std::string& scheme() const { return (scheme_ ? *scheme_ : empty_str_); };
    const std::string& user_info() const { return (user_info_ ? *user_info_ : empty_str_); };
    const std::string& host() const { return (host_ ? *host_ : empty_str_); };
    const std::string& port() const { return (port_ ? *port_ : empty_str_); };
    const std::string& path() const { return (path_ ? *path_ : empty_str_); };
    const std::string& query() const { return (query_ ? *query_ : empty_str_); };
    const std::string& fragment() const { return (fragment_ ? *fragment_ : empty_str_); };


  private:
    int validity_state_;
    const std::string empty_str_;

    typedef utils::maybe<std::string> Component;
    Component scheme_;
    Component user_info_;
    Component host_;
    Component port_;
    Component path_;
    Component query_;
    Component fragment_;

    void Validate_();
    void ParseScheme_(const std::string& raw_uri, size_t& raw_uri_pos);
    void ParseUserInfo_(const std::string& raw_uri, size_t& raw_uri_pos);
    void ParseHost_(const std::string& raw_uri, size_t& raw_uri_pos);
    void ParsePort_(const std::string& raw_uri, size_t& raw_uri_pos);
    void ParsePath_(const std::string& raw_uri, size_t& raw_uri_pos);
    void ParseQuery_(const std::string& raw_uri, size_t& raw_uri_pos);
    void ParseFragment_(const std::string& raw_uri, size_t& raw_uri_pos);

    // normalize components: decode safe-to-decode chars, convert encoded hex to upper, remove dot
    // segments and collapse slashes in path
    void Normalize_();
    utils::maybe<std::string> PercentDecode_(const std::string& str,
                                             const char* dont_decode_set = NULL) const;
    std::string PercentEncode_(const std::string& str, const char* dont_encode_set = NULL) const;
    void ConvertEncodedHexToUpper_(std::string& str);
    utils::maybe<std::string> RemoveDotSegments_(const std::string& str) const;
    std::string CollapseSlashes_(const std::string& str) const;

    // helpers:
    void RemoveLastSegment_(std::string& path) const;
    void MoveSegmentToOutput_(std::string& input, std::string& output) const;

    void ValidateScheme_();
    void ValidateHost_();
    void ValidatePort_();
    void ValidatePath_();
    void ValidateQuery_();
    bool IsEncodedOctet_(const char* str) const;
};

std::ostream& operator<<(std::ostream& out, const RqTarget& RqTarget);

}  // namespace http

#endif  // WS_HTTP_URI_H
