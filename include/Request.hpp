#pragma once
#include <string>
#include <iostream>
#include <vector>
#include <cctype>
#include <algorithm>
#include <iostream>
#include <iterator>
#include <map>
#include <sstream>
#include <utility>
#include <stdexcept>
#include "Configuration.hpp"

enum requestState
{
	RECEIVING,
	RECEIVED,
	PROCESSED,
	DONE
};

enum parsingState
{
	REQUEST_LINE,
	HEADERS,
	BODY,
	PARSING_DONE
};

class Request
{
private:
	int 								method;
	std::string							uri;
	std::string							version;
	std::map<std::string, std::string>	headers;
	std::string							body;
	int									parsingState;
	int									state;
	
	// parsing
	void	parseRequestLine(const std::string &line);
	void	setParsingState(int state);
	void	setUri(const std::string &str);
	void	setMethod(const std::string &str);
	void	setVersion(const std::string &str);
	void	setHeaders(std::stringstream &stream);
	bool	isValidHeader(const std::string &line, std::string &name, std::string &value);
	void	parseHeaderName(const std::string &str, std::string &name);
	void	parseHeaderValue(const std::string &str, std::string &value);
	void	parseBody(std::stringstream &stream);
	bool	hasBody();

public:
	Request();
	~Request();
	
	// getters
	int									getMethod() const;
	std::string							getUri() const;
	std::string 						getVersion() const;
	std::map<std::string, std::string>	getHeaders() const;
	std::string 						getBody() const;
	void								setRequestState(int state);
	int 								getRequestState();
	int 								getParsingState();
	void								clearRequest();
	bool								isKeepAlive();
	void								parseRequest(std::stringstream &stream);
	std::string							getHost() const;
	int 								getPort() const;
};