#ifndef WS_HTTP_H
#define WS_HTTP_H

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

class Uri {
  public:
    Uri(const std::string& raw_uri);
    Uri(const std::string& scheme, const std::string& host, unsigned short port, const std::string& path, const std::string& query, const std::string& fragment);
    Uri(const Uri& other);
    Uri& operator=(const Uri& other);
    bool operator==(const Uri& other) const;
    ~Uri() {}
    bool Good() const;
    bool Bad() const;
    int ErrorCode() const;
    std::string ToStr() const;

  private:
    enum State {
        URI_GOOD = 0,
        URI_TOO_LONG = 414,
        URI_OTHER_ERROR = 1
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

};

}  // namespace http

#endif  // WS_HTTP_H
