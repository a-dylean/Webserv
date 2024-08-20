#pragma once
#include <string>
#include <iostream>
#include <map>
#include <vector>
#include <cctype>
#include <sstream>

// Request       = Request-Line              ; Section 5.1
// *(( general-header        ; Section 4.5
//  | request-header         ; Section 5.3
//  | entity-header ) CRLF)  ; Section 7.1
// CRLF
// [ message-body ]          ; Section 4.3


// PEST Grammar (src: https://pest.rs/)
// file = { SOI ~ (delimiter | request)* ~ EOI}

// request = {	
// 	request_line ~
//     headers? ~
//     NEWLINE ~
//     body?
// }

// request_line = _{ method ~ " "+ ~ uri ~ " "+ ~ "HTTP/" ~ version ~ NEWLINE }
// uri = { (!whitespace ~ ANY)+ }
// method = { ("GET" | "DELETE" | "POST" | "PUT") }
// version = { (ASCII_DIGIT | ".")+ }
// whitespace = _{ " " | "\t" }

// headers = { header+ }
// header = { header_name ~ ":" ~ whitespace ~ header_value ~ NEWLINE }
// header_name = { (!(NEWLINE | ":") ~ ANY)+ }
// header_value = { (!NEWLINE ~ ANY)+ }

// body = { (!delimiter ~ ANY)+ }
// delimiter = { "#"{3} ~ NEWLINE+ }

class Request
{
private:
	std::string buffer;
	std::string method;
	std::string uri;
	std::string version;
	std::map<std::string, std::string> headers;
	std::vector<char> body;

public:
	Request(std::string buffer);
	~Request();
	//getters
	std::string getBuffer() const;
	std::string getMethod() const;
	std::string getUri() const;
	std::string getVersion() const;
	std::map<std::string, std::string> getHeaders() const;
	std::vector<char> getBody() const;
	//parsing
	bool parseRequest();
	bool parseRequestLine(const std::string &line);
    bool parseUri(const std::string &str, std::string &uri);
    bool parseMethod(const std::string &str, std::string &method);
    bool parseVersion(const std::string &str, std::string &version);
    bool parseWhitespace(char c);
    bool parseHeaders(const std::vector<std::string> &lines);
    bool parseHeader(const std::string &line, std::string &name, std::string &value);
    bool parseHeaderName(const std::string &str, std::string &name);
    bool parseHeaderValue(const std::string &str, std::string &value);
	bool parseBody(const std::string &str, std::vector<char> &body);
    bool parseDelimiter(const std::string &str);
	//debug
	void printRequest(Request &req);
};