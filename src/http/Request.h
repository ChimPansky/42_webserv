#ifndef WS_HTTP_REQUEST_H
#define WS_HTTP_REQUEST_H

#include <map>
#include <string>
#include <vector>

namespace http {
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
enum RqStatus {
    RQ_INCOMPLETE,
    RQ_BAD,
    RQ_GOOD
};

class RequestBuilder;
class Request {
  public:
    Request();

  private:
    RqStatus status_;
    Method method_;
    std::string uri_;  // todo: change to struct/class
    Version version_;
    // bool bad_request;
    // bool rq_complete;
    std::map<std::string, std::string> headers_;
    std::vector<char> body_;

  public:
    std::string GetHeaderVal(const std::string& key) const;
    void Print() const;
    friend class RequestBuilder;

    // Getters:
    RqStatus status() const;
    Method method() const;
    const std::string& uri() const;
    Version version() const;
    const std::map<std::string, std::string>& headers() const;
    const std::vector<char>& body() const;
};

}  // namespace http

#endif  // WS_HTTP_REQUEST_H

// headers related to body:

// Content-Type: application/json
// Content-Length
// Transfer-Encoding: chunked
// Content-Encoding: gzip // Describes any compression applied to the request body, such as gzip, so the server knows how to decode it before
// Content-Disposition: form-data; name="file"; filename="example.txt" //Provides additional information about how to process the data in the body, often used in multipart/form-data for file uploads. It specifies details like the name of the file or field.
// Expect: 100-continue // This header is used when the client wants the server to check something before sending the body. For example, Expect: 100-continue is used when the client wants to ensure the server is ready before sending a large body.
// Content-Range: bytes 200-1000/67589 // Used to specify that only a portion of the data is being sent, often in cases where uploads are sent in parts.
// Content-Language: en-US // Indicates the language of the content in the request body.
// Content-MD5: Q2hlY2sgSW50ZWdyaXR5IQ== // Contains a base64-encoded MD5 checksum of the request body. This is used to ensure the integrity of the data being transmitted.
// Content-Location: /example-resource // Provides an alternate location for the data being sent in the request body. This is more commonly seen in responses, but it can be used in requests to indicate the source of the body content.
// TE: gzip, chunked // Specifies what transfer codings (like chunked, gzip) the client is willing to accept in the response. It can also relate to the request body if it's expecting certain types of transfer encodings for the request.
// Trailer: Expires // Specifies that some headers are sent in the trailing part of the chunked message, after the body. This header can be used in requests that send chunked body content.
// Authorization: Bearer <token>
// Proxy-Authorization: Bearer <token> // These headers aren't directly related to the body but are used to provide authentication credentials for requests that may include sensitive data in the body.
