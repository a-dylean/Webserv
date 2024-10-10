# Webserv
This project is about writing HTTP server.

https://www.notion.so/Webserv-e0b101573b614f959497d4856d97e59c

Todo List - Webserv 
	
- <s>determine the main classes or structs we’ll need throughout the project</s>
- <s>determine how we’ll divide the work</s>	
- <s>have a basic http request working (without a configuration file yet)</s>
- <s>understand how configuration files work (Mostafa) and need to be parsed</s>
- <s>understand how to listen on all clients and ports with select(), poll(0 or epoll() (Victoire)</s>
- <s>understand how CGI works and is used (Victoire)</s>
- <s>how to parse the http request and create response (Alisa)</s>
- create a map / schema for all processes (all)
- <s>add exceptions instead of returning error and exiting</s>
- <s>correctly rename all vars and functions</s>

NEW TODOS:
- Parsing:
	- add the first (default) location block if there's no "location /" block set in the config
- Errors handling:
	- Proper status code everywhere (all requests) and verify that correct error pages are returned (default or custom)
	- <s>Setup default error pages (add a file with them)</s>
	- Sometimes we return 500 error page and the status code 404...
- Connecting config and request:
	- <s>Check host, port and server_names and have server block</s>
	- <s>Inside server block check for location path and return either corrponding location or default location</s>

- Directives:
	- <s>Server names</s>
	- <s>Error pages</s>
	- <s>Limit client body size</s>
	- <s>HTTP methods</s>
	- HTTP redirection
	- <s>Autoindex: turn on or off directory listing (add to the whole process)</s>
	- <s>The index directive (specifies the index file name(s) to be served when a directory is requested. By default, Nginx looks for an index.html file in the root directory)</s>
	- <s>Alias (Define a directory or a file from where the file should be searched (for example, if url /kapouet is rooted to /tmp/www, url /kapouet/pouic/toto/pouet is /tmp/www/pouic/toto/pouet))</s>
	- CGI (see bellow)
- CGI part:
	- add a checkif there's a cgi pass
	- run cgi code if there's a path // handle by nginx if there's no path
- Creating final response string for each request:
	- Set required headers dynamically for all requests
	- Verify that the correct body is set

- Signals to handle + valgrind checks to add

- Decide which files we want to serve to demonstrate functionality of webserv:
	- a fully static website
	- upload files functionality
	- GET, POST, and DELETE methods
	- multiple ports

- Redirection
	- using a return directive in file.conf block the server

- be able to send a very big request (sometime with a big URI) and check that the server can handle it

**Main parts of the project:**
1. Server logic (loop)
2. HTTP request (parsing)
3. Configuration file (parsing)
4. Creating response (analyze config and request)
5. CGI


Error cases:

- When we launch the program and try to upload twice in a row it breaks everything (error 404 everywhere)

- Search for all read/recv/write/send on a socket and check that, if an error is returned, the client is removed.

- Limit the client body (use: curl -X POST -H "Content-Type: plain/text" --data "BODY IS HERE write something shorter or longer than body limit").

- In the configuration, try to setup the same port multiple times. It should not work.

- Launch multiple servers at the same time with different configurations but with common ports. Does it work? If it does, ask why the server should work if one of the configurations isn't functional. Keep going.

- You can use a script containing an infinite loop or an error; you are free to do whatever tests you want within the limits of acceptability that remain at your discretion. The group being evaluated should help you with this. METTRE UN TIMEOUT SUR LE SCRIPT POUR PAS RESTER BLOQUE

- Check that when we accept connections we don't have more clients than the maximum number of clients allowed. (here we never check that NewClientFd is not > than the max number of clients)