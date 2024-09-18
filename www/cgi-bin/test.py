#!/usr/bin/env python3

# import os
# import cgi
# import cgitb
# import json

# # Enable debugging
# cgitb.enable()

# # Content-Type header for the HTML response
# print("Content-Type: text/html\n")

# # Determine the request method (GET, POST, DELETE)
# request_method = os.environ.get('REQUEST_METHOD')
# content_length = os.environ.get('CONTENT_LENGTH')
# print("content_length: ", content_length)
# # request_query = os.environ.get('QUERY_STRING')
# # print(request_query)
# # Initialize form data
# form = cgi.FieldStorage()

# print("METHOD")
# print(request_method)
# if request_method == '0':
#     print("<html><body><h1>Received get</h1>")
#     print("<h2>")
#     # print(request_query)
#     print("</h2></body></html>")


# elif request_method == '1':
#     print("<html><body><h1>Comment Added</h1><h2>")
#     print("</h2></body></html>")

# elif request_method == '2':
#     print("<html><body>")
#     print("<h1>DELETE</h1>")
#     print("</body></html>")

# else:
#     print("<html><body><h1>Method Not Supported</h1></body></html>")


import os
import sys
import cgi
import html

# Print the necessary HTTP headers
print("Content-Type: text/html\n")

# Retrieve the POST data (length is provided in CONTENT_LENGTH environment variable)
if os.environ.get("REQUEST_METHOD", "") == "POST":
    content_length = int(os.environ.get("CONTENT_LENGTH", 0))
    if content_length > 0:
        # Read the input from stdin (which contains the POST body)
        post_data = sys.stdin.read(content_length)
        print(post_data)
        # Parse the POST data
        form = cgi.FieldStorage(fp=sys.stdin, environ=os.environ, keep_blank_values=True)

        # Get the value of the "comment" field
        comment = form.getvalue("comment", "")

        # Make sure to escape HTML to avoid XSS
        comment = html.escape(comment)

        # Append the comment to a file (to simulate saving the comment)
        with open("comments.txt", "a") as f:
            f.write(comment + "\n")

# Read previously saved comments
comments = []
if os.path.exists("comments.txt"):
    with open("comments.txt", "r") as f:
        comments = f.readlines()

# Display the comments and the form
print("""
<html>
<head>
    <title>Comments Page</title>
</head>
<body>
    <h1>Comments</h1>
    <ul>
""")

# Output all the comments
for comment in comments:
    print(f"<li>{html.escape(comment.strip())}</li>")

print("""
    </ul>
    <form method="POST" action="/cgi-bin/comments.py">
        <textarea name="comment" rows="4" cols="50"></textarea><br>
        <input type="submit" value="Post Comment">
    </form>
</body>
</html>
""")
