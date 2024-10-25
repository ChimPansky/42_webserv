#ifndef WS_HTTP_URI_H
#define WS_HTTP_URI_H

#include <string>
namespace http {

class Uri {
  public:
    Uri(const std::string& raw_uri);
    Uri(const std::string& path, const std::string& query, const std::string& fragment);
    Uri(const Uri& other);
    ~Uri() {}

    Uri& operator=(const Uri& other);
    bool operator==(const Uri& other) const;
    bool operator!=(const Uri& other) const;

    bool Good() const { return status_ == URI_GOOD_BIT; };
    bool Fail() const { return (status_ & (URI_BAD_BIT | URI_FAIL_BIT)) != 0; };
    int ErrorCode() const;
    std::string ToStr() const;

    const std::string& path() const { return path_; };
    const std::string& query() const { return query_; };
    const std::string& fragment() const { return fragment_; };

  private:
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

    UriStatus status_;

    size_t raw_uri_pos_;

    std::string path_; // "/", "/index.html", "/path/to/file"
    std::string query_; // consider using a map; "?key1=val1&key2=val2"
    std::string fragment_; // used to jump to specific location on website, e.g. "#section1", "#details", "#dashboard"

    void Validate_();
    void ParseRawUri_(const std::string& raw_uri);
    void ParsePath_(const std::string& raw_uri, ParseState& state);
    void ParseQuery_(const std::string& raw_uri, ParseState& state);
    void ParseFragment_(const std::string& raw_uri, ParseState& state);

    // helpers:
    bool EndOfRawUri_(const std::string& raw_uri) const;

    bool IsValidPathChar_(char c) const;
    bool IsValidQueryOrFragmentChar_(char c) const;

    bool IsValidPath_(const std::string& path) const;
    bool IsValidQuery_(const std::string& query) const;
    bool IsValidFragment_(const std::string& fragment) const;
};
} // namespace http

#endif // WS_HTTP_URI_H
