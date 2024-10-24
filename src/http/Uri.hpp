#ifndef WS_HTTP_URI_H
#define WS_HTTP_URI_H

#include <string>
namespace http {

class Uri {
  public:
    Uri(const std::string& raw_uri);
    Uri(const std::string& scheme, const std::string& host, unsigned short port, const std::string& path, const std::string& query, const std::string& fragment);
    Uri(const Uri& other);
    Uri& operator=(const Uri& other);
    bool operator==(const Uri& other) const;
    bool operator!=(const Uri& other) const;
    ~Uri() {}
    bool Good() const { return status_ == URI_GOOD_BIT; };
    bool Bad() const { return (status_ & URI_BAD_BIT) != 0; };
    bool Eof() const { return (status_ & URI_EOF_BIT) != 0; };
    bool Fail() const { return (status_ & (URI_BAD_BIT | URI_FAIL_BIT | URI_BAD_SCHEME_BIT)) != 0; };
    int ErrorCode() const;
    std::string ToStr() const;

    const std::string& scheme() const { return scheme_; };
    const std::string& host() const { return host_; };
    unsigned short port() const { return port_; };
    const std::string& path() const { return path_; };
    const std::string& query() const { return query_; };
    const std::string& fragment() const { return fragment_; };

  private:
    enum UriStatus {
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

    enum ParseState {
        PS_SCHEME,
        PS_HOST,
        PS_PORT,
        PS_PATH,
        PS_QUERY,
        PS_FRAGMENT,
        PS_END
    };

    ParseState state_;
    UriStatus status_;

    size_t raw_uri_pos_;

    std::string scheme_; // "http", "https"
    std::string host_; // "www.example.com", "192.168.1.1"
    unsigned short port_; // 80, 443
    std::string path_; // "/", "/index.html", "/path/to/file"
    std::string query_; // consider using a map; "?key1=val1&key2=val2"
    std::string fragment_; // used to jump to specific location on website, e.g. "#section1", "#details", "#dashboard"


    void Validate_();
    void ParseRawUri_(const std::string& raw_uri);
    void ParseScheme_(const std::string& raw_uri);
    void ParseHost_(const std::string& raw_uri);
    void ParsePort_(const std::string& raw_uri);
    void ParsePath_(const std::string& raw_uri);
    void ParseQuery_(const std::string& raw_uri);
    void ParseFragment_(const std::string& raw_uri);


    // helpers:
    bool EndOfRawUri_(const std::string& raw_uri) const;

    bool IsValidHostChar_(char c) const;
    bool IsValidPathChar_(char c) const;
    bool IsValidQueryOrFragmentChar_(char c) const;

    bool IsValidHost_(const std::string& host) const;
    bool IsValidPath_(const std::string& path) const;
    bool IsValidQuery_(const std::string& query) const;
    bool IsValidFragment_(const std::string& fragment) const;
};
} // namespace http

#endif // WS_HTTP_URI_H
