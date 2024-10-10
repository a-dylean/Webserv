#include "Request.hpp"

Request::Request() : state(RECEIVING), parsingState(REQUEST_LINE) {}

Request::~Request() {};

int Request::getMethod() const
{
    return method;
}

std::string Request::getUri() const
{
    return uri;
}

std::string Request::getVersion() const
{
    return version;
}

std::map<std::string, std::string> Request::getHeaders() const
{
    return headers;
}

std::string Request::getHost() const
{
    std::map<std::string, std::string>::const_iterator it = headers.find("Host");
    if (it != headers.end())
    {
        std::string host = it->second.substr(0, it->second.find(':'));
        return host;
    }
    return "";
}

int Request::getPort() const
{
    std::map<std::string, std::string>::const_iterator it = headers.find("Host");
    if (it != headers.end())
    {
        size_t pos = it->second.find(':');
        if (pos != std::string::npos)
        {
            std::string portValue = it->second.substr(pos + 1);
            int port;
	        std::stringstream ss(portValue);
	        ss >> port;
            return (port);
        }
    }
    return 80;
}

std::string Request::getBody() const
{
    return body;
}

void Request::setUri(const std::string &str)
{
    for (size_t i = 0; i < str.size(); ++i)
    {
        if (std::isspace(str[i]))
            throw std::logic_error("Invalid URI");
        this->uri += str[i];
    }
}

void Request::setMethod(const std::string &str)
{
    if (str == "GET")
        this->method = GET;
    else if (str == "POST")
        this->method = POST;
    else if (str == "DELETE")
        this->method = DELETE;
    else
        this->method = UNKNOWN;
}

void Request::setVersion(const std::string &str)
{
    if (str.find("HTTP/") == 0)
    {
        this->version = str;
    }
    else
    {
        throw std::logic_error("Invalid HTTP version");
    }
}

void Request::setHeaders(std::stringstream &stream)
{
    std::string line;
    while (std::getline(stream, line))
    {
        if (!line.empty() && line[line.size() - 1] == CR)
        {
            line.erase(line.size() - 1);
        }

        if (line.empty())
        {
            setParsingState(BODY);
            break;
        }
        std::string name, value;
        if (isValidHeader(line, name, value))
        {
            headers[name] = value;
        }
    }

    if (parsingState != BODY)
    {
        setParsingState(HEADERS);
    }
}

bool Request::isValidHeader(const std::string &line, std::string &name, std::string &value)
{
    size_t pos = line.find(':');
    if (pos == std::string::npos)
        return false;
    std::string header_name = line.substr(0, pos);
    std::string header_value = line.substr(pos + 2);
    try
    {
        parseHeaderName(header_name, name);
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
    }
    try
    {
        parseHeaderValue(header_value, value);
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
    }
    return true;
}

void Request::parseHeaderName(const std::string &str, std::string &name)
{
    for (size_t i = 0; i < str.size(); ++i)
    {
        if (str[i] == LF || str[i] == ':')
            throw std::logic_error("Invalid header");
        name += str[i];
    }
}

void Request::parseHeaderValue(const std::string &str, std::string &value)
{
    for (size_t i = 0; i < str.size(); ++i)
    {
        if (str[i] == LF)
            throw std::logic_error("Invalid header value");
        value += str[i];
    }
}

void Request::parseBody(std::stringstream &stream)
{
    if (!hasBody())
    {
        setParsingState(PARSING_DONE);
        return;
    }
    std::string content_length = headers["Content-Length"];
    std::stringstream ss(content_length);
    int len;
    ss >> len;
    std::string new_body;
    for (int i = 0; i < len; i++)
    {
        char c;
        stream.get(c);
        if (stream.eof())
            break;
        new_body += c;
    }
    this->body += new_body;
    if ((int)this->body.size() == len)
    {
        setParsingState(PARSING_DONE);
    }
}
void Request::parseRequestLine(const std::string &line)
{
    std::istringstream stream(line);
    std::string method, uri, version;

    stream >> method;
    try
    {
        setMethod(method);
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
    }

    stream >> uri;
    try
    {
        setUri(uri);
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
    }

    stream >> version;
    try
    {
        setVersion(version);
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
    }
}
bool Request::hasBody()
{
    return headers.find("Content-Length") != headers.end();
}

void Request::setParsingState(int state)
{
    this->parsingState = state;
}

int Request::getParsingState()
{
    return this->parsingState;
}
void Request::parseRequest(std::stringstream &stream)
{
    std::string line;
    if (parsingState == REQUEST_LINE)
    {
        std::getline(stream, line);
        parseRequestLine(line);
        setParsingState(HEADERS);
    }
    if (parsingState == HEADERS)
        setHeaders(stream);
    if (parsingState == BODY)
    {
        parseBody(stream);
    }
}

void Request::setRequestState(int state)
{
    this->state = state;
}

int Request::getRequestState()
{
    return state;
}

void Request::clearRequest(void)
{
    this->method = 0;
    this->uri.clear();
    this->headers.clear();
    this->state = 0;
    this->body.clear();
    this->parsingState = REQUEST_LINE;
}
