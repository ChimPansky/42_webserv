# Common Gateway Interface (CGI) Documentation

## Overview of CGI

The Common Gateway Interface (CGI) is a standard protocol that defines how web servers communicate with executable scripts to generate dynamic content. When a request is received, the server can pass information to a script, execute it, and return the output to the client as a response. This allows for dynamic content generation and interactive web applications.

## How CGI is Implemented in Our Server

### Identifying CGI Requests

- **CGI Request Recognition**: A request is identified as a CGI request if the URI ends with `/cgi-bin/` followed by the script name, which may optionally include an additional path.
- **Script Validation**: The script name must immediately follow the `/cgi-bin/` segment. Requests that do not specify a script directly after `/cgi-bin/` are considered invalid, and the server returns a **400 Bad Request** error.
- **PATH\_INFO Compliance**: Any extra path information provided after the script name is passed to the script as `PATH_INFO`, in accordance with the CGI 1.1 specification.

#### Examples:

- Valid Paths:
  - `/test/cgi-bin/script.py`
  - `/test/cgi-bin/script.py/additional/path`
  - `/cgi-bin/script.py`
- Invalid Path:
  - `/cgi-bin/test/script.py`

### Configuration of CGI Directories

- **CGI Directory**: The server identifies CGI requests based on the `/cgi-bin/` directory. This directory is reserved for executable scripts.
- **Customizability**: While other servers may use file extensions (e.g., `.cgi`) or custom directories for CGI detection, our implementation strictly relies on the `/cgi-bin/` directory.

### Configuration Block for CGI Requests

In the server configuration file, CGI requests are defined using a location block for the `/cgi-bin/` directory:

```plaintext
location /cgi-bin/ {
  ...
}
```

The location block can also be defined as /cgi-bin. By default, if no `cgi_extensions` are specified, the server handles `.py` and `.php` extensions.

### Execution Environment

- **Execution Context**: CGI scripts are executed in the directory where they are located. This ensures that relative path access within the script works correctly.

### Supported Script Extensions

- In the server's configuration file, the `cgi_extensions` directive specifies the allowed extensions for CGI scripts.
- **Supported Extensions**:
  - `.py` (Python)
  - `.pl` (Perl)
  - `.php` (PHP)
  - `.sh` (Bash)

#### Example Configuration:

```plaintext
cgi_extensions: .py, .pl, .php, .sh
```

Scripts with extensions outside this list are not executed, and the server returns a **403 Forbidden** error if such a script is requested.

### Handling HTTP Methods

- **GET and POST Support**: The server supports both GET and POST methods for CGI requests.
- **Chunked Requests**: For chunked POST requests, the server unchunks the data before passing it to the CGI script. The CGI script expects EOF (end of file) as the end of the body.
- **Output Handling**: If the CGI script does not return a `Content-Length` header, the server treats EOF as the end of the returned data.

#### Example Behavior:

- A POST request with chunked data is unchunked by the server before execution.
- If no `Content-Length` is provided in the CGI output, the server reads until EOF.

### Content-Type Requirement

- If an entity body is returned by the CGI script, it **MUST** include a `Content-Type` field in the response, as specified in the CGI 1.1 documentation.

## Additional Information

- **Environment Variables**: CGI relies on environment variables to pass information to the script. Commonly used variables include:

  - `SCRIPT_NAME`: The name of the script being executed.
  - `PATH_INFO`: Extra path information provided after the script name.
  - `QUERY_STRING`: The query parameters from the URI.
  - `REMOTE_ADDR`: The IP address of the client.
  - `CONTENT_TYPE`: The MIME type of the request body (for POST requests).

- **Error Handling**:

  - **400 Bad Request**: Returned if the request does not specify a valid script after `/cgi-bin/`.
  - **500 Internal Server Error**: Returned for execution errors within the script.
  - **501 Not Implemented**: Returned if the script has an unsupported extension.


By adhering to these guidelines, our server ensures robust and predictable CGI behavior while maintaining compatibility with the CGI 1.1 standard.
