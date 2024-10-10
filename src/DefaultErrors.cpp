#include "DefaultErrors.hpp"


std::map<int, std::string> create_error_pages_map() {
    std::map<int, std::string> error_pages;
    error_pages.insert(std::make_pair(400, http_error_400_page));
    error_pages.insert(std::make_pair(401, http_error_401_page));
    error_pages.insert(std::make_pair(402, http_error_402_page));
    error_pages.insert(std::make_pair(403, http_error_403_page));
    error_pages.insert(std::make_pair(404, http_error_404_page));
    error_pages.insert(std::make_pair(405, http_error_405_page));
    error_pages.insert(std::make_pair(406, http_error_406_page));
    error_pages.insert(std::make_pair(408, http_error_408_page));
    error_pages.insert(std::make_pair(409, http_error_409_page));
    error_pages.insert(std::make_pair(410, http_error_410_page));
    error_pages.insert(std::make_pair(411, http_error_411_page));
    error_pages.insert(std::make_pair(412, http_error_412_page));
    error_pages.insert(std::make_pair(413, http_error_413_page));
    error_pages.insert(std::make_pair(414, http_error_414_page));
    error_pages.insert(std::make_pair(415, http_error_415_page));
    error_pages.insert(std::make_pair(416, http_error_416_page));
    error_pages.insert(std::make_pair(421, http_error_421_page));
    error_pages.insert(std::make_pair(429, http_error_429_page));
    error_pages.insert(std::make_pair(494, http_error_494_page));
    error_pages.insert(std::make_pair(495, http_error_495_page));
    error_pages.insert(std::make_pair(496, http_error_496_page));
    error_pages.insert(std::make_pair(497, http_error_497_page));
    error_pages.insert(std::make_pair(500, http_error_500_page));
    error_pages.insert(std::make_pair(501, http_error_501_page));
    error_pages.insert(std::make_pair(502, http_error_502_page));
    error_pages.insert(std::make_pair(503, http_error_503_page));
    error_pages.insert(std::make_pair(504, http_error_504_page));
    error_pages.insert(std::make_pair(505, http_error_505_page));
    error_pages.insert(std::make_pair(507, http_error_507_page));
    return error_pages;
}

std::string getStatusMsg(int code)
{
	switch (code)
	{
	case 200:
		return "OK";
	case 201:
		return "Created";
	case 202:
		return "Accepted";
	case 204:
		return "No Content";
	case 307:
		return "Temporary Redirect";
	case 400:
		return "Bad Request";
	case 401:
		return "Unauthorized";
	case 403:
		return "Forbidden";
	case 404:
		return "Not Found";
	case 405:
		return "Method Not Allowed";
	case 409:
		return "Conflict";
	case 411:
		return "Length Required";
	case 413:
		return "Payload Too Large";
	case 415:
		return "Unsupported Media Type";
	case 500:
		return "Internal Server Error";
	case 504:
		return "Gateway Timeout";
	case 501:
		return "Not Implemented";
	case 505:
		return "HTTP Version Not Supported";
	}
	return "Internal Server Error";
}