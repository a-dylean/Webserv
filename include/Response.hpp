#pragma once
#include <string>
#include <map>
#include "Configuration.hpp"
#include "Request.hpp"
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include "DefaultErrors.hpp"
#include "Cgi.hpp"
#include "DefaultPages.hpp"

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

	LocationBlock getLocationFromServer(Configuration &config, Request &req);
	void bodySizeCheck(Configuration &config, LocationBlock &location);
	void methodCheck(LocationBlock location);
	void setErrorBody(LocationBlock location);
	std::string getBodyFromFile(std::string path);
	void getBody(std::string uri, LocationBlock location);
	std::string handleRedirection(LocationBlock &location);

public:
	Response();
	Response(Request &req);
	~Response();

	std::string getResponse(Configuration &config);
	void setStatusLine();
	void createResponseStr(LocationBlock location);
	void handleUploadFiles(LocationBlock &location, Request &req);
	void handleGetRequest(LocationBlock location);
	void handlePostRequest(LocationBlock location);
	void handleDeleteRequest(LocationBlock location);
	void setHeaders(LocationBlock location);
	void setMimeType(std::string const &fileName);
	void setStatusCode(int code);
	void setBody(std::string const &body);
	void clearResponse();
};

std::string intToString(int value);
int stringToInt(const std::string &str);
bool isDirectory(const std::string &path);
bool isFile(const std::string &path);
int serverBlocksCount(Configuration &config, std::string host, int port);
ServerBlock getDefaultServerBlock(Configuration &config, std::string host, int port);
bool matchExists(Configuration &config, std::string host, int port);
ServerBlock getMatchingServerBlock(Configuration &config, std::string host, int port);
bool locationBlockExists(ServerBlock serverBlock, std::string uri);
LocationBlock getMatchingLocationBlock(ServerBlock ServerBlock, std::string uri);
bool serverBlockExists(Configuration &config, Request &req);
std::string getDefaultErrorBody(int statusCode);
bool isInIndex(std::string fileName, LocationBlock location);
std::string getFilePath(std::string path, std::string fileName);
bool hasDefaultFile(const std::string &directoryPath, LocationBlock location);
std::string setPath(LocationBlock location, std::string uri);
std::string generateDirectoryListingHTML(const std::string &directoryPath, const std::string &rootPath);
std::string setFileCopyName(std::string givenName);
int checkIfFileExists(const std::string &dirPath, std::string fileName);
std::string getContentType(const std::string &contentType);
std::string getFileBody(std::string body, std::string &boundary);
std::string getFileContent(std::string body, Request &req);
int getNbBoundaries(std::string body, std::string boundary);
void changeDirBack(std::string path);