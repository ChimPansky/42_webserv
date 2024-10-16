
# HTTP Requests

## Parsing Method:

## Parsing URI:

## Parsing Version:

## Parsing Headers

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
