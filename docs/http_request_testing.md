
# HTTP Requests Testing

## Curl:
example usage:
- curl 127.0.0.1:8081
- curl -v 127.0.0.1:8081 (Verbose option; show contents of the request sent)
- curl 127.0.0.1:8081 --output - (output server response in terminal)

## NC:
example usages:
- echo -e "GET /folder1/folder2/ HTTP/2\r\n   " | nc 127.0.0.1 8081
- < request.txt nc 127.0.0.1 8081
- nc 127.0.0.1 8081
  - use standard input (problem: cannot send \r manually...)

## Browser:

Open your favourite Browser and enter the IP:Port/URI in the address bar
