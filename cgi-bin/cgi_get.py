#!/usr/bin/env python3
import os
import cgi
import cgitb
import json

request_method = os.environ.get('REQUEST_METHOD')
request_query = os.environ.get('QUERY_STRING')
html_content = """<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Comment Form</title>
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
        input[type="submit"]:hover {
            background-color: #45a049;
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
<body>

<h1>Leave a Comment</h1>

<div class="comment-block">
    <form action="/cgi_post.py" method="post">
        <label for="name">Name:</label>
        <input type="text" id="name" name="name" required>
        
        <label for="comment">Comment:</label>
        <textarea id="comment" name="comment" required></textarea>
        
        <input type="submit" value="Submit Comment">
    </form>
</div>
</body>
</html>
"""
print(html_content)
print("Content-Type: text/html")
# to test for infinite loop
# while True:
#     print("hello")