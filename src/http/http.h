#ifndef WS_HTTP_H
#define WS_HTTP_H

#include <sstream>
#include <string>

namespace http {

#define RQ_LINE_LEN_LIMIT 8192
#define RQ_URI_LEN_LIMIT 8192

enum Method {
    HTTP_NO_METHOD,
    HTTP_GET,
    HTTP_POST,
    HTTP_DELETE
};
enum Version {  // probably only need to handle Ver_1_0 and Ver_1_1
    HTTP_NO_VERSION,
    HTTP_0_9,
    HTTP_1_0,
    HTTP_1_1,
    HTTP_2,
    HTTP_3
};

// todo: move this to utils ?
std::string UnsignedShortToStr(unsigned short num) {
    std::stringstream ss;
    ss << num;
    return ss.str();
}

class Uri {
  public:
    Uri(const std::string& raw_uri);
    Uri(const std::string& scheme, const std::string& host, unsigned short port, const std::string& path, const std::string& query, const std::string& fragment);
    Uri(const Uri& other);
    Uri& operator=(const Uri& other);
    bool operator==(const Uri& other) const;
    bool operator!=(const Uri& other) const;
    ~Uri() {}
    bool Good() const { return state_ == URI_GOOD_BIT; };
    bool Bad() const { return (state_ & URI_BAD_BIT) != 0; };
    bool Eof() const { return (state_ & URI_EOF_BIT) != 0; };
    bool Fail() const { return (state_ & (URI_BAD_BIT | URI_FAIL_BIT | URI_BAD_SCHEME_BIT)) != 0; };
    int ErrorCode() const;
    std::string ToStr() const;

    const std::string& scheme() const { return scheme_; };
    const std::string& host() const { return host_; };
    unsigned short port() const { return port_; };
    const std::string& path() const { return path_; };
    const std::string& query() const { return query_; };
    const std::string& fragment() const { return fragment_; };

  private:
    enum State {
        URI_GOOD_BIT = 0,
        URI_BAD_BIT = 1L << 0,
        URI_EOF_BIT = 1L << 2,
        URI_TOO_LONG_BIT = 1L << 3,
        URI_BAD_SCHEME_BIT = 1L << 4,
        URI_BAD_HOST_BIT = 1L << 5,
        URI_BAD_PORT_BIT = 1L << 6,
        URI_BAD_PATH_BIT = 1L << 7,
        URI_BAD_QUERY_BIT = 1L << 8,
        URI_BAD_FRAGMENT_BIT = 1L << 9,
        URI_FAIL_BIT = 1L << 16
    };

    std::string scheme_; // "http", "https"
    std::string host_; // "www.example.com", "192.168.1.1"
    unsigned short port_; // 80, 443
    std::string path_; // "/", "/index.html", "/path/to/file"
    std::string query_; // consider using a map; "?key1=val1&key2=val2"
    std::string fragment_; // used to jump to specific location on website, e.g. "#section1", "#details", "#dashboard"

    State state_;

    void ParseStr_(const std::string& raw_uri);
    void Validate_();

    // helpers:
    bool IsValidHostChar_(char c) const;
    bool IsValidPathChar_(char c) const;
    bool IsValidQueryOrFragmentChar_(char c) const;

    bool IsValidHost_(const std::string& host) const;
    bool IsValidPath_(const std::string& path) const;
    bool IsValidQuery_(const std::string& query) const;
    bool IsValidFragment_(const std::string& fragment) const;
};

}  // namespace http

#endif  // WS_HTTP_H
