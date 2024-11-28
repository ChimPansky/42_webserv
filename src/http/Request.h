#ifndef WS_HTTP_REQUEST_H
#define WS_HTTP_REQUEST_H

#include <map>
#include <string>
#include <vector>

#include <http.h>
#include <ResponseCodes.h>
#include <Uri.h>

namespace http {

class RequestBuilder;
enum RqStatus {
    RQ_END_OF_HEADERS_NOT_REACHED = 0,
    RQ_END_OF_HEADERS_REACHED,
    RQ_BAD,
    RQ_END_OF_BODY_REACHED
};

enum BodyType {
    BT_NONE = 0,
    BT_REGULAR,
    BT_CHUNKED
};

class RequestBody {
  public:
    RequestBody();

    BodyType type() const { return type_; }
    const std::vector<char>& content() const { return content_; }
    bool ready_to_send_to_server( ) const { return ready_to_send_to_server_; }

  private:
    BodyType type_;
    std::vector<char> content_;
    bool ready_to_send_to_server_;
};

class Request {
  public:
    Request();

    void ProcessNewData(std::vector<char>& buf);
    bool BodyExpected() const { return body_.type() != BT_NONE; }

    std::string ToString() const;

    std::pair<bool /*header-key found*/, std::string /*header-value*/> GetHeaderVal(
        const std::string& key) const;
    RqStatus status() const { return status_; }
    ResponseCode rs_code() const { return rs_code_; }
    Method method() const { return method_; }
    const http::Uri& uri() const { return uri_; }
    Version version() const { return version_; }
    const RequestBody& body() const { return body_; }

    void set_status(RqStatus status) { status_ = status; }
    void set_rs_code(ResponseCode rs_code) { rs_code_ = rs_code; }
    void set_method(Method method) { method_ = method; }
    void set_uri(const http::Uri& uri) { uri_ = uri; }
    void set_version(Version version) { version_ = version; }
    void add_header(const std::string& key, const std::string& value) {
        headers_[key] = value;
    }

  private:
    RqStatus status_;
    ResponseCode rs_code_;
    Method method_;
    http::Uri uri_;
    Version version_;
    std::map<std::string, std::string> headers_;
    RequestBody body_;

};

}  // namespace http

#endif  // WS_HTTP_REQUEST_H

// headers related to body:

// Content-Type: application/json
// Content-Length
// Transfer-Encoding: chunked
// Content-Encoding: gzip // Describes any compression applied to the request body, such as gzip, so
// the server knows how to decode it before Content-Disposition: form-data; name="file";
// filename="example.txt" //Provides additional information about how to process the data in the
// body, often used in multipart/form-data for file uploads. It specifies details like the name of
// the file or field. Expect: 100-continue // This header is used when the client wants the server
// to check something before sending the body. For example, Expect: 100-continue is used when the
// client wants to ensure the server is ready before sending a large body. Content-Range: bytes
// 200-1000/67589 // Used to specify that only a portion of the data is being sent, often in cases
// where uploads are sent in parts. Content-Language: en-US // Indicates the language of the content
// in the request body. Content-MD5: Q2hlY2sgSW50ZWdyaXR5IQ== // Contains a base64-encoded MD5
// checksum of the request body. This is used to ensure the integrity of the data being transmitted.
// Content-Location: /example-resource // Provides an alternate location for the data being sent in
// the request body. This is more commonly seen in responses, but it can be used in requests to
// indicate the source of the body content. TE: gzip, chunked // Specifies what transfer codings
// (like chunked, gzip) the client is willing to accept in the response. It can also relate to the
// request body if it's expecting certain types of transfer encodings for the request. Trailer:
// Expires // Specifies that some headers are sent in the trailing part of the chunked message,
// after the body. This header can be used in requests that send chunked body content.
// Authorization: Bearer <token>
// Proxy-Authorization: Bearer <token> // These headers aren't directly related to the body but are
// used to provide authentication credentials for requests that may include sensitive data in the
// body.
