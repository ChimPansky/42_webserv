import cgi, cgitb

# val = input("Enter your value: ")
# print(val)

form = cgi.FieldStorage()
print(form)
#username = form["username"].value
# emailaddress = form["emailaddress"].value

print("Content-Type: text/html\r\n\r\n")

print("<html>")
print("<head>")
print("<title> MY FIRST CGI FILE </title>")
print("</head>")
print("<body>")
print("<h3> This is HTML's Body Section </h3>")
# print(username)
# print(emailaddress)
print("</body>")
print("</html>")
