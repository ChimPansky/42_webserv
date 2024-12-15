#!/usr/bin/env python3

import sys

# Correct CGI header
print("Content-Type: text/html")
print()

# Intentional infinite loop
while True:
    pass  # Loop forever

# This part won't execute because of the infinite loop
print("<html>")
print("<head><title>Infinite Loop Test</title></head>")
print("<body>")
print("<h1>This is a test CGI script with an infinite loop</h1>")
print("</body>")
print("</html>")
