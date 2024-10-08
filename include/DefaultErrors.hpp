#pragma once
#include "Response.hpp"
#include "Configuration.hpp"
#include <string>

std::map<int, std::string> create_error_pages_map();
std::string getStatusMsg(int code);

const std::map<int, std::string> http_error_pages = create_error_pages_map();

const std::string http_error_400_page =
"<html>" CRLF
"<head><title>400 Bad Request</title></head>" CRLF
"<body>" CRLF
"<center><h1>400 Bad Request</h1></center>" CRLF
"</body>" CRLF
"</html>" CRLF;

const std::string http_error_401_page =
"<html>" CRLF
"<head><title>401 Authorization Required</title></head>" CRLF
"<body>" CRLF
"<center><h1>401 Authorization Required</h1></center>" CRLF
"</body>" CRLF
"</html>" CRLF;

const std::string http_error_402_page =
"<html>" CRLF
"<head><title>402 Payment Required</title></head>" CRLF
"<body>" CRLF
"<center><h1>402 Payment Required</h1></center>" CRLF
"</body>" CRLF
"</html>" CRLF;

const std::string http_error_403_page =
"<html>" CRLF
"<head><title>403 Forbidden</title></head>" CRLF
"<body>" CRLF
"<center><h1>403 Forbidden</h1></center>" CRLF
"</body>" CRLF
"</html>" CRLF;

const std::string http_error_404_page =
"<html>" CRLF
"<head><title>404 Not Found</title></head>" CRLF
"<body>" CRLF
"<center><h1>404 Not Found</h1></center>" CRLF
"</body>" CRLF
"</html>" CRLF;

const std::string http_error_405_page =
"<html>" CRLF
"<head><title>405 Method Not Allowed</title></head>" CRLF
"<body>" CRLF
"<center><h1>405 Method Not Allowed</h1></center>" CRLF
"</body>" CRLF
"</html>" CRLF;

const std::string http_error_406_page =
"<html>" CRLF
"<head><title>406 Not Acceptable</title></head>" CRLF
"<body>" CRLF
"<center><h1>406 Not Acceptable</h1></center>" CRLF
"</body>" CRLF
"</html>" CRLF;

const std::string http_error_408_page =
"<html>" CRLF
"<head><title>408 Request Time-out</title></head>" CRLF
"<body>" CRLF
"<center><h1>408 Request Time-out</h1></center>" CRLF
"</body>" CRLF
"</html>" CRLF;

const std::string http_error_409_page =
"<html>" CRLF
"<head><title>409 Conflict</title></head>" CRLF
"<body>" CRLF
"<center><h1>409 Conflict</h1></center>" CRLF
"</body>" CRLF
"</html>" CRLF;

const std::string http_error_410_page =
"<html>" CRLF
"<head><title>410 Gone</title></head>" CRLF
"<body>" CRLF
"<center><h1>410 Gone</h1></center>" CRLF
"</body>" CRLF
"</html>" CRLF;

const std::string http_error_411_page =
"<html>" CRLF
"<head><title>411 Length Required</title></head>" CRLF
"<body>" CRLF
"<center><h1>411 Length Required</h1></center>" CRLF
"</body>" CRLF
"</html>" CRLF;

const std::string http_error_412_page =
"<html>" CRLF
"<head><title>412 Precondition Failed</title></head>" CRLF
"<body>" CRLF
"<center><h1>412 Precondition Failed</h1></center>" CRLF
"</body>" CRLF
"</html>" CRLF;

const std::string http_error_413_page =
"<html>" CRLF
"<head><title>413 Request Entity Too Large</title></head>" CRLF
"<body>" CRLF
"<center><h1>413 Request Entity Too Large</h1></center>" CRLF
"</body>" CRLF
"</html>" CRLF;

const std::string http_error_414_page =
"<html>" CRLF
"<head><title>414 Request-URI Too Large</title></head>" CRLF
"<body>" CRLF
"<center><h1>414 Request-URI Too Large</h1></center>" CRLF
"</body>" CRLF
"</html>" CRLF;

const std::string http_error_415_page =
"<html>" CRLF
"<head><title>415 Unsupported Media Type</title></head>" CRLF
"<body>" CRLF
"<center><h1>415 Unsupported Media Type</h1></center>" CRLF
"</body>" CRLF
"</html>" CRLF;

const std::string http_error_416_page =
"<html>" CRLF
"<head><title>416 Requested Range Not Satisfiable</title></head>" CRLF
"<body>" CRLF
"<center><h1>416 Requested Range Not Satisfiable</h1></center>" CRLF
"</body>" CRLF
"</html>" CRLF;

const std::string http_error_421_page =
"<html>" CRLF
"<head><title>421 Misdirected Request</title></head>" CRLF
"<body>" CRLF
"<center><h1>421 Misdirected Request</h1></center>" CRLF
"</body>" CRLF
"</html>" CRLF;

const std::string http_error_429_page =
"<html>" CRLF
"<head><title>429 Too Many Requests</title></head>" CRLF
"<body>" CRLF
"<center><h1>429 Too Many Requests</h1></center>" CRLF
"</body>" CRLF
"</html>" CRLF;

const std::string http_error_494_page =
"<html>" CRLF
"<head><title>400 Request Header Or Cookie Too Large</title></head>" CRLF
"<body>" CRLF
"<center><h1>400 Bad Request</h1></center>" CRLF
"<center>Request Header Or Cookie Too Large</center>" CRLF
"</body>" CRLF
"</html>" CRLF;

const std::string http_error_495_page =
"<html>" CRLF
"<head><title>400 The SSL certificate error</title></head>" CRLF
"<body>" CRLF
"<center><h1>400 Bad Request</h1></center>" CRLF
"<center>The SSL certificate error</center>" CRLF
"</body>" CRLF
"</html>" CRLF;

const std::string http_error_496_page =
"<html>" CRLF
"<head><title>400 No required SSL certificate was sent</title></head>" CRLF
"<body>" CRLF
"<center><h1>400 Bad Request</h1></center>" CRLF
"<center>No required SSL certificate was sent</center>" CRLF
"</body>" CRLF
"</html>" CRLF;

const std::string http_error_497_page =
"<html>" CRLF
"<head><title>400 The plain HTTP request was sent to HTTPS port</title></head>" CRLF
"<body>" CRLF
"<center><h1>400 Bad Request</h1></center>" CRLF
"<center>The plain HTTP request was sent to HTTPS port</center>" CRLF
"</body>" CRLF
"</html>" CRLF;

const std::string http_error_500_page =
"<html>" CRLF
"<head><title>500 Internal Server Error</title></head>" CRLF
"<body>" CRLF
"<center><h1>500 Internal Server Error</h1></center>" CRLF
"</body>" CRLF
"</html>" CRLF;

const std::string http_error_501_page =
"<html>" CRLF
"<head><title>501 Not Implemented</title></head>" CRLF
"<body>" CRLF
"<center><h1>501 Not Implemented</h1></center>" CRLF
"</body>" CRLF
"</html>" CRLF;

const std::string http_error_502_page =
"<html>" CRLF
"<head><title>502 Bad Gateway</title></head>" CRLF
"<body>" CRLF
"<center><h1>502 Bad Gateway</h1></center>" CRLF
"</body>" CRLF
"</html>" CRLF;

const std::string http_error_503_page =
"<html>" CRLF
"<head><title>503 Service Temporarily Unavailable</title></head>" CRLF
"<body>" CRLF
"<center><h1>503 Service Temporarily Unavailable</h1></center>" CRLF
"</body>" CRLF
"</html>" CRLF;

const std::string http_error_504_page =
"<html>" CRLF
"<head><title>504 Gateway Time-out</title></head>" CRLF
"<body>" CRLF
"<center><h1>504 Gateway Time-out</h1></center>" CRLF
"</body>" CRLF
"</html>" CRLF;

const std::string http_error_505_page =
"<html>" CRLF
"<head><title>505 HTTP Version Not Supported</title></head>" CRLF
"<body>" CRLF
"<center><h1>505 HTTP Version Not Supported</h1></center>" CRLF
"</body>" CRLF
"</html>" CRLF;

const std::string http_error_507_page =
"<html>" CRLF
"<head><title>507 Insufficient Storage</title></head>" CRLF
"<body>" CRLF
"<center><h1>507 Insufficient Storage</h1></center>" CRLF
"</body>" CRLF
"</html>" CRLF;