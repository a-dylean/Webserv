#pragma once
#include <string>
#include <map>
#include "Configuration.hpp"
#include "Request.hpp"
#include <dirent.h>
#include <unistd.h> 
#include <sys/stat.h>
#include <string.h>

class Configuration;
class Request;

class Response
{
private:
	Request req;
	std::string statusLine;
	int statusCode;
	std::string headers;
	std::string body;
	std::string response;
	std::string mimeType;
	std::string getStatusMsg(int code);
	void setBody(std::string const &body);
	void processServerBlock(Configuration &config, Request &req);
	std::string getPath(std::vector<ServerBlock>::iterator it, std::string uri);
	void handleRoot(std::string configPath, std::string requestUri);
	LocationBlock getLocationBlock(std::vector<ServerBlock>::iterator it, std::string uri);
	bool locationBlockExists(std::vector<ServerBlock>::iterator it, std::string uri);
	void handleDirectory(std::string path);
	void handleFile(std::string path);
public:
	Response();
	Response(Request &req);
	~Response();
	std::string getResponse(Configuration &config);
	void setStatusLine();
	void createResponseStr();
	void handleGetRequest(Configuration &config);
	void handlePostRequest(Configuration &config);
	void handleDeleteRequest(Configuration &config);
	void setHeaders();
	void setMimeType(std::string const &fileName);
	void handleErrorPage(std::vector<ServerBlock>::iterator it);
	void setErrorBody(std::ifstream &file, std::stringstream &body);
};


bool isDirectory( const std::string & path );
bool isFile( const std::string & path );