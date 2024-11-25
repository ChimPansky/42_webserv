#ifndef WS_HTTP_RQ_TARGET_H
#define WS_HTTP_RQ_TARGET_H

#include <cstddef>
#include <string>
#include <numeric_utils.h>
namespace http {

/* RqTarget class:
 Request Target is the string that comes after the method and before the version in the request line. Example: "GET /path/to/file?query1=1&query2=2 HTTP/1.1"
 We need to handle 2 kinds of targets:
    1. Absolute path: /path[?query] (slash followed by zero or more path segments and an optional query). example: /path/to/file?query1=1&query2=2
    2. Absolute URI: scheme://host[:port]/path[?query] (scheme followed by host followed by optional port followed by absolute path followed by optional query. example: http://www.example.com/path/to/file?query1=1&query2=2

    - Mostly we handle RqTargets in first form, but according to RFC we also need to be able to accept Requests with Targets in second form (Requests to Proxy servers).
    - The host and port information in the second form will then override the value of the host header field and will be used to choose a Server from the ServerCluster.

    in order to make the target comparable to other targets, we need to normalize it. Normalization includes:
    - Store scheme and host in lower-case since they are case-insensitive.
    - Percent-decode the host, path and query components. Only decode unreserved characters, leave reserved characters (chars that are used as delimiters like %2F for /, %3F for ?,...) as is (might need to change this later - host and query may have different decoding rules).
    - Make the encoded hex characters in the path and query components uppercase.
    - Remove dot-segments from the path component.
    - Collapse sequences of slashes in the path-component.

*/

class RqTarget {
  public:
    enum RqTargetStatus {
        RQ_TARGET_GOOD = 0,
        RQ_TARGET_TOO_LONG = 1L << 3,  //todo: check if we need to implement this here or in RqBuilder
        RQ_TARGET_BAD_SCHEME = 1L << 4,
        RQ_TARGET_HAS_USER_INFO = 1L << 5,
        RQ_TARGET_BAD_HOST = 1L << 6,
        RQ_TARGET_BAD_PORT = 1L << 7,
        RQ_TARGET_BAD_PATH = 1L << 8,
        RQ_TARGET_BAD_QUERY = 1L << 9,
        RQ_TARGET_HAS_FRAGMENT = 1L << 10,
    };
    RqTarget(){};
    RqTarget(const std::string& raw_uri);
    RqTarget(const std::string& scheme, const std::string& user_info, const std::string& host, const std::string& port, const std::string& path, const std::string query, const std::string& fragment);
    RqTarget(const RqTarget& rhs);
    ~RqTarget() {}
    RqTarget& operator=(const RqTarget& rhs);
    bool operator==(const RqTarget& rhs) const;
    bool operator!=(const RqTarget& rhs) const;

    int validity_state() const { return validity_state_; };
    bool Good() const { return validity_state_ == RQ_TARGET_GOOD; };

    bool HasScheme() const { return scheme_.first; };
    bool HasUserInfo() const { return user_info_.first; };
    bool HasHost() const { return host_.first; };
    bool HasPort() const { return port_.first; };
    bool HasPath() const { return path_.first; };
    bool HasQuery() const { return query_.first; };
    bool HasFragment() const { return fragment_.first; };
    std::string ToStr() const;
    std::string GetDebugString() const;

    const std::string& scheme() const { return scheme_.second; };
    const std::string& user_info() const { return user_info_.second; };
    const std::string& host() const { return host_.second; };
    const std::string& port() const { return port_.second; };
    const std::string& path() const { return path_.second; };
    const std::string& query() const { return query_.second; };
    const std::string& fragment() const { return fragment_.second; };


  private:

    static const char* kUnreserved;
    static const char* kGenDelims;
    static const char* kSubDelims;
    int validity_state_;

    typedef std::pair<bool /*component_defined*/, std::string /*value*/> Component;
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

    // normalize components: decode safe-to-decode chars, convert encoded hex to upper, remove dot segments and collapse slashes in path
    void Normalize_();
    std::pair<bool/*valid_triplet*/, std::string> PercentDecode_(const std::string& str, const char* decode_set = kUnreserved) const;
    void ConvertEncodedHexToUpper_(std::string& str);
    std::pair<bool /*no_directory_traversal*/, std::string> RemoveDotSegments_(const std::string& str) const;
    std::string CollapseSlashes_(const std::string& str) const;

    // helpers:
    void RemoveLastSegment_(std::string& path) const;
    void MoveSegmentToOutput_(std::string& input, std::string& output) const;

    void ValidateScheme_();
    void ValidateHost_();
    void ValidatePort_();
    void ValidatePath_();
    void ValidateQuery_();
    bool IsEncodedOctet_(const char *str) const;
    bool IsUnreservedChar_(char c) const;
};

std::ostream& operator<<(std::ostream& out, const RqTarget& RqTarget);

}  // namespace http

#endif  // WS_HTTP_URI_H
