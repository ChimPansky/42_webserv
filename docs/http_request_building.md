
# HTTP Requests

HTTP-Requests are read piece by piece (chunk by chunk) by the ClientSocket::Recv(std::vector<char>& buf) function
We call this function with RequestBuilder.rq_buf_.data() {
    ClientSocket::Recv() is calling the <sys/socket.h> recv(_socket_fd, void* _buf, size_t _n, int _flags) function {
        Here we read ClientSocket.sock_buf_sz bytes into the ClientSocket.sock_buf_
    }
    Then the bytes from ClientSocket.sock_buf_ are appended to the vector        RequestBuilder.rq_buf_
}

After everytime a piece (chunk) has been attached to buffer in the RequestBuilder (through ClientSession::ClientSocket::Recv()), ClientSession::ProcessNewData() is called where ClientSession.RequestBuilder::ParseChunk() is called.
In RequestBuilder::ParseChunk() the Request is analized step by step and information about http::Method, URI, Version, Headers, Body are extracted from the buffer.

## Parsing Method:

## Parsing URI:

## Parsing Version:

## Parsing Headers

As soon as Host header has been parsed successfully, we know everything we need to determine which Server the Request is intended for (We already knew IP and Port, because thats what the Socket is listening on, now we also know the name of the server (Host header) and the location (URI)).
So we will set the RequestBuilder& server_ GetServer(IP, Port, Host, URI)...


## **HTTP-Method**: The method indicates the action to be performed on the resource. Examples include `GET`, `POST`, `PUT`, `DELETE`, etc.


## **Request-URI**: The URI (Uniform Resource Identifier) identifies the resource on the server.

## **HTTP-Version**: Specifies the HTTP protocol version, typically `HTTP/1.1`.

## **Headers**: Key-value pairs that convey additional information about the request, such as `Host`, `User-Agent`, `Content-Type`, etc.

mandatory headers (if one is missing -> Error 400):
- Host (in HTTP/1.1 and up)
- Content-Type (if there is a message body, e.g. POST, PUT, PATCH methods)
- Content-Length (if there is a message body, unless Transfer-Encoding header exists)
- Authorization (when accessing resource that requires authentication)
- Cookie (required if request is part of a session or carries session data)



## **Body**: An optional component that contains the data being sent to the server (used in methods like `POST` and `PUT`).

### Example of an HTTP GET Request

```http
GET /index.html HTTP/1.1
Host: www.example.com
User-Agent: Mozilla/5.0
Accept: text/html
```

### Example of an HTTP POST Request

```http
POST /submit-form HTTP/1.1
Host: www.example.com
Content-Type: application/x-www-form-urlencoded
Content-Length: 27

name=John&age=30
```

### Syntax Validation Rules for HTTP Requests:

- **Request Line**:
  - Must start with a valid HTTP method (e.g., `GET`, `POST`, `PUT`, etc.).
  - The URI must be valid and may include query parameters.
  - The HTTP version must be correctly formatted, e.g., `HTTP/1.1`.

- **Headers**:
  - Each header must be on a new line.
  - Headers must follow the `Name: Value` format, where the header name must be valid and the value must not be empty.

- **Body**:
  - If a body is present, it should match the `Content-Type` specified in the headers.
  - The `Content-Length` header, if present, must correctly reflect the size of the body.
