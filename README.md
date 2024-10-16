# Webserv
<img width="446" alt="Screenshot 2024-04-13 222920" src="www/Screenshot 2024-09-09 105335.png">

## Overview

This project is part of 42 school common core cursus. Webserv is a custom HTTP server inspired by [NGINX](https://github.com/nginx/nginx) and designed to handle HTTP requests and responses, including support for CGI scripts, file uploads, and most common HTTP methods. The project aims to provide a fully functional web server with configurable settings and error handling.

## Features

- **HTTP Methods**: Supports GET, POST and DELETE methods. Sending other types of request will result in 405 Method Not Allowed Error.
- **File Uploads**: Handles file uploads to specified directories.
- **CGI Support**: Executes CGI scripts for dynamic content.
- **Configuration**: Customizable server settings via configuration files.
- **Error Handling**: Default error pages for various HTTP status codes. Custom error pages can be defined as well.
- **Multiple Ports**: Listens on multiple ports as specified in the configuration.

## Directory Structure

- **bin/**: Compiled object files
- **cgi-bin/**: CGI scripts
- **config/**: Configuration files
- **include/**: Header files
- **src/**: Source files
- **www/**: Web content

## Configuration

The server configuration is specified in `.conf` files located in the `config` directory. The default configuration file is `default.conf`.
### Directives
<table>
  <tr>
    <th>Directive</th>
    <th >Syntax</th>
    <th>Example(s)</th>
    <th>Default</th>
    <th>Context</th>
    <th>Description</th>
  </tr>
  <tr>
    <td>Listen</td>
    <td><code>listen</code></td>
    <td><code>listen localhost:8080;</code>, <code>listen 127.0.0.1:8081;</code>, <code>listen 8082;</code></td>
    <td><code>localhost:8080</code></td>
    <td><code>server</code></td>
    <td>Sets host address and port for IP on which the server will accept requests. Both address and port, or only address or only port can be specified. An address may also be a hostname.</td>
  </tr>
  <tr>
    <td>Server name</td>
    <td><code>server_name</code></td>
    <td><code>server_name example.com;</code></td>
    <td>-</td>
    <td><code>server</code></td>
    <td>Sets names of a virtual server. The first name becomes the primary server name.</td>
  </tr>
  <tr>
    <td>Root</td>
    <td><code>root</code></td>
    <td><code>root ./www;</code></td>
    <td><code>./www</code></td>
    <td><code>server</code>, <code>location</code></td>
    <td>Sets the root directory for requests.</td>
  </tr>
  <tr>
    <td>Default error pages</td>
    <td><code>error_page</code></td>
    <td><code>error_page 404 /404.html;</code> <code>error_page 403 /403.html;</code></td>
    <td>-</td>
    <td><code>server</code>, <code>location</code></td>
    <td>Defines the URI to redirect to in case of a specified error code.</td>
  </tr>
  <tr>
    <td>Client body size limit</td>
    <td><code>client_max_body_size</code></td>
    <td><code>client_max_body_size 1k;</code></td>
    <td><code>client_max_body_size 1m;</code></td>
    <td><code>server</code>, <code>location</code></td>
    <td>Sets the maximum allowed size of the client request body.</td>
  </tr>
  <tr>
    <td>Allowed HTTP methods</td>
    <td><code>allowed_methods</code></td>
    <td><code>allowed_methods GET|POST|DELETE;</code></td>
    <td>-</td>
    <td><code>server</code>, <code>location</code></td>
    <td>Defines a list of accepted HTTP methods for the route.</td>
  </tr>
  <tr>
    <td>Directory listing</td>
    <td><code>autoindex</code></td>
    <td><code>autoindex on;</code></td>
    <td><code>off</code></td>
    <td><code>server</code>, <code>location</code></td>
    <td>Turns on or off directory listing.</td>
  </tr>
  <tr>
    <td>HTTP redirection</td>
    <td><code>return</code></td>
    <td><code>return 301 http://example.com;</code></td>
    <td>-</td>
    <td><code>server</code>, <code>location</code></td>
    <td>Stops processing and redirects to a specified source, returning a specified code to the client.</td>
  </tr>
  <tr>
    <td>Index file</td>
    <td><code>index</code></td>
    <td><code>index new_index.html;</code></td>
    <td><code>index.html</code></td>
    <td><code>server</code>, <code>location</code></td>
    <td>Sets a default file to answer if the request is a directory.</td>
  </tr>
  <tr>
    <td>Alias</td>
    <td><code>alias</code></td>
    <td><code>alias /var/www/html;</code></td>
    <td>-</td>
    <td><code>location</code></td>
    <td>Defines a directory or a file from where the file should be searched (a replacement for the specified location).</td>
  </tr>
  <tr>
    <td>CGI</td>
    <td><code>cgi</code></td>
    <td><code>cgi .py .php;</code></td>
    <td>-</td>
    <td><code>server</code>, <code>location</code></td>
    <td>Specifies file extension(s) based on which CGI script can be executed for a given location.</td>
  </tr>
  <tr>
    <td>Uploaded files location</td>
    <td><code>upload_location</code></td>
    <td><code>upload_location ./www/upload;</code></td>
    <td><code>./www/upload</code></td>
    <td><code>location</code></td>
    <td>Makes the route able to accept uploaded files and configures where they should be saved.</td>
  </tr>
</table>

### Example Configuration file

```conf
server {
	listen 8080;
	server_name localhost;
	client_max_body_size 1k;

    location / {
	allowed_methods GET|POST|DELETE;
	root ./www;
	autoindex on;
    }

    location /upload {
	allowed_methods POST|DELETE;
	upload_location ./www/upload;
    }
}
```

## Building the Project

To build the project, use the provided Makefile. Run the following command in the project root directory:

```sh
make
```

This will compile the source files and generate the `webserv` executable.

## Running the Server

To run the server with the default configuration:

```sh
./webserv
```

To specify a custom configuration file:

```sh
./webserv path/to/config.conf
```

## Handling Requests

### GET Request

Handles static files and directory listings (if autoindex is enabled).

### POST Request

Handles form submissions and file uploads.

### DELETE Request

Deletes specified files from the server.

## Error Handling

Custom error pages can be specified in the configuration file. Default error pages are provided for common HTTP status codes.

## CGI Support

CGI scripts can be executed for dynamic content. The server will handle the execution of the script and return the output as the response.

## Authors

- [Alisa Tonkopiy](https://github.com/a-dylean)
- [Mostafa Omrane](https://github.com/CodingOnBush)
- [Victoire Vaudaine](https://github.com/vicvdn)

## Useful resources

- [Notion page](https://www.notion.so/Webserv-e0b101573b614f959497d4856d97e59c) with summary we created while working on the project
- [Another Notion pabe by sgah and vdescham](https://webserv42.notion.site/Webserv-cbb6ab4136ba4b4c8cb4f98109d5fc1f)
- [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/html/)
- [Sockets and Network Programming in C](https://www.codequoi.com/en/sockets-and-network-programming-in-c/)
- [HTTP request and response and how web applications work](https://codesensei.medium.com/http-request-and-response-and-how-web-applications-work-76780d4cb14c)

## Additional tips

For better understanding of the project, first of all, we recommend installing and configuring NGINX. It will help you to understand how a web server works and how to configure it.\
As you explore resources with code examples, we encourage you to implement these examples yourself. Take the time to thoroughly understand each step by referring to the official documentation and man pages.
