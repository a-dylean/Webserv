#!/usr/bin/env python3
import os
import cgi
import cgitb
import urllib.parse

# Enable CGI error reporting
cgitb.enable()

request_method = os.environ.get('REQUEST_METHOD')
request_query = os.environ.get('QUERY_STRING')

# HTML content starts here
html_content = """<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>POST Success</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            margin: 20px;
        }
        form {
            margin-top: 20px;
        }
        label {
            display: block;
            margin: 8px 0 4px;
        }
        input[type="text"], textarea {
            width: 90%;
            padding: 15px;
            margin-bottom: 10px;
            border: 1px solid #ccc;
            border-radius: 4px;
        }
        input[type="submit"] {
            background-color: #4CAF50;
            color: white;
            padding: 10px 15px;
            border: none;
            border-radius: 4px;
            cursor: pointer;
        }
        h1 {
            animation: bounce 1s infinite;
        }
        @keyframes bounce {
            0%, 20%, 50%, 80%, 100% {
                transform: translateY(0);
            }
            40% {
                transform: translateY(-20px);
            }
            60% {
                transform: translateY(-10px);
            }
        }
        .comment-block {
            border: 1px solid #ccc;
            border-radius: 4px;
            padding: 15px;
            margin-top: 20px;
            background-color: #f9f9f9;
        }
    </style>
</head>
"""

def parse_query_string(query_string):
    # Decode the query string and extract name and comment
    parsed = urllib.parse.parse_qs(query_string)
    name = parsed.get('name', [None])[0]
    comment = parsed.get('comment', [None])[0]
    return name, comment

def printPost():
    name, comment = parse_query_string(request_query)
    print(html_content)
    print("<body><h1>Comment successfully submitted!</h1>")
    print(f"""
    <div class="comment-block">
        <strong>Name:</strong> {name}<br><br>
        <strong>Comment:</strong><br>
        <p>{comment}</p>
    </div>
    """)
    print("</body></html>")

print("Content-Type: text/html")  # Set the content type
print()  # Blank line to end headers
printPost()
