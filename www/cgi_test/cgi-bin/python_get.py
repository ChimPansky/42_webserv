import cgi, cgitb

import os
import sys

def eprint(*args, **kwargs):
    print(*args, file=sys.stderr, **kwargs)

for key, value in os.environ.items():
    eprint(f"{key}: {value}")


form = cgi.FieldStorage()

username = form["username"].value
emailaddress = form["emailaddress"].value

print("Content-Type: text/html\n")

print("<html>")
print("<head>")
print("<title> MY FIRST CGI FILE </title>")
print("</head>")
print("<body>")
print("<h3> This is HTML's Body Section </h3>")
print(username)
print(emailaddress)
print("</body>")
print("</html>")
