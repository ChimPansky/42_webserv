#ifndef WS_HTTP_REQUEST_H
#define WS_HTTP_REQUEST_H

#include <ResponseCodes.h>
#include <RqTarget.h>
#include <file_utils.h>
#include <http.h>

#include <map>
#include <string>

namespace http {

class RequestBuilder;

enum BodyStorageType {
    BST_IN_TMP_FOLDER,
    BST_ON_SERVER
};

struct BodyStorage {
    BodyStorage();
    ~BodyStorage();
    std::string path;
    BodyStorageType storage_type;
    bool transfer_complete;
};

struct Request {
    Request();

    bool has_body() const;

    ResponseCode status;
    Method method;
    http::RqTarget rqTarget;
    Version version;
    std::map<std::string, std::string> headers;
    BodyStorage body;

    utils::maybe<std::string> GetHeaderVal(const std::string& key) const;
    std::string GetDebugString() const;
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
