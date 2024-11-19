Source: RFC 9112

URI:

```
         foo://example.com:8042/over/there?name=ferret#nose
         \_/   \______________/\_________/ \_________/ \__/
          |           |            |            |        |
       scheme     authority       path        query   fragment
          |   _____________________|__
         / \ /                        \
         urn:example:animal:ferret:nose
```

**Clarification of terms**:

* **URI** (Unified Resource Identifier): the thing that is typed into the address bar of a browser; the argument of curl,...
* **Request-Target**: Generally a subset of the URI.
* **Origin-Server**: Source of requested content. Connection can be established directly by client or through Proxy-Server. in the URI "www.example.com/page1" the server would be "www.example.com"
* **Proxy-Server**: Intermediary between client and Origin-Server. Will Receive Request from client, (potentially) modify it and forward it to Origin-Server. Why proxies? Caching, Security, Load Balancing,...
* **Scheme:**
  `scheme      = ALPHA *( ALPHA / DIGIT / "+" / "-" / "." )`
  "http", "https", "ftp", "file", "irc",... Always followed by ":"
* **Authority**:
  `authority   = [ userinfo "@" ] host [ ":" port ]`

  * **Userinfo**:
    `userinfo    = *( unreserved / pct-encoded / sub-delims / ":" )`
  * **Host:**
    `IPvFuture   = "v" 1*HEXDIG "." 1*( unreserved / sub-delims / ":" )`

    `host        = (IPv6address / IPvFuture) / IPv4address / reg-name`
  * **Port**:
    port          = *DIGIT
* **Path:**
  `path-abempty = *( "/" segment ) `                              ; begins with "/" or is empty
  `path-absolute = "/" [ segment-nz * ( "/" segment ) ]` ; begins with "/" but not "//"
  `path-rootless`	`= segment-nz *( "/" segment )`	  ; begins with a segment
  `path-empty = 0 `						  ; zero characters
* **Query**:
  `query         = *( pchar / "/" / "?" )`
* **Fragment**:
  `fragment      = *( pchar / "/" / "?" )`
* **Absolute-URI**:
  `gen-delims = ":" / "/" / "?" / "#" / "[" / "]" / "@"`
  `sub-delims = "!" / "$" / "&" / "'" / "(" / ")" / "*" / "+" / "," / ";" / "="`
  `unreserved = ALPHA / DIGIT / "-" / "." / "_" / "~"`
  `reserved = gen-delims / sub-delims `
  `pct-encoded   = "%" HEXDIG HEXDIG`
  `pchar = unreserved / pct-encoded / sub-delimbs / ":" / "@"`
  `segment = *pchar`
  `segment-nz = 1*pchar`
  `hier-part = "//" authority path-abempty / path-absolute / path-rootless / path-empty`
  `absolute-URI  = scheme ":" hier-part [ "?" query ]`
* Relative Reference:
  `relative-ref  = relative-part [ "?" query ] [ "#" fragment ]`
  `relative-part = "//" authority path-abempty / path-absolute / path-noscheme / path-empty```
* Absolut-Path:
  an absolute path is a relative reference that begins with a single slash character.

 
A request-line begins with a method token, followed by a single space (SP), the request-target, and another single space (SP), and ends with the protocol version.



`request-line   = method SP request-target SP HTTP-version`

Request-Target is **NOT** the same as URI!

The Request-Target can be in one of 4 forms:

| RFC-Reference | Form-Type                      | Description                                                                                                                                                                                                                                                                                                                                  | Format & Example(s)                                                                                    |
| ------------- | ------------------------------ | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------ |
| RFC9912/3.2.1 | origin-form<br />(most common) | For requests to origin-servers. All requests to origin-servers<br />(except methods CONNECT/OPTIONS) have to use this form.                                                                                                                                                                                                                  | origin-form    = absolute-path [ "?" query ]<br />"GET /where?q=now HTTP/1.1"<br />"GET / HTTP/1.1"    |
| RFC9912/3.2.2 | absolute-form                  | For requests to proxy-servers. All requests to origin-servers<br />(except methods CONNECT/OPTIONS) have to use this form.<br />The <br />Even though it's not intended, Origin-Servers **MUST** **accept** absolute-form.<br />The host of targets in absolute-form **overrides** the value in the host-header field. | absolute-form  = absolute-URI<br />"GET http://www.example.org/pub/WWW/<br />TheProject.html HTTP/1.1" |
| RFC9912/3.2.3 | authority-form                 | Only for CONNECT method. Request-target consists of only host + port.                                                                                                                                                                                                                                                                       | authority-form = uri-host ":" port<br />"CONNECT www.example.com:7070"                                 |
| RFC9912/3.2.4 | asterisk-form                  | Only for OPTIONS method. Request-target == "*".<br />Used to retrieve a list of supported Methods from server.                                                                                                                                                                                                                               | asterisk-form  = "*"<br />"OPTIONS * HTTP/1.1"                                                         |

Since we only need to handle GET/POST/DELETE methods, we can already discard authority- and asterisk-form.
Even though we are implementing a origin-server, we still need to be able to accept request-targets in absolute-form;
Best way to do this is to extract the host portion from the request-target and override the value of the host-header field.

When parsing the request-target string (the string between method and version in the first line of request), we should proceed as follows:
TODO: adapt process below...

1. separate request-target into (scheme, authority, host, port,) path, query and fragment
2. ProcessPath():

   1. DecodePercentages()
   2. CheckForInvalidPathChars()
   3. Normalize():
      1. multiple "///" collapse to "/"
      2. "./" can be removed
      3. "../" deletes previous folder from uri (work with stack containing of strings that represent a folder each. if "../" leads to level that is above root -> ERROR (path traversing...)
3. ProcessQuery():

   1. DecodePercentages()
   2. CheckForInvalidQueryChars()
4. ProcessFragment()

   1. DecodePercentages()
   2. CheckForInvalidFramentChars()
