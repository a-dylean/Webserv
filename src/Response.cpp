#include "Response.hpp"
#include "Webserv.hpp"

Response::Response() : statusCode(0) {};

Response::Response(Request &req) : req(req), statusCode(0) {};

Response::~Response() {};

void Response::setStatusCode(int code)
{
	this->statusCode = code;
}

void Response::setBody(std::string const &body)
{
	this->body = body;
}

void Response::setErrorBody(LocationBlock location)
{
	if (location.errorPages.empty() || location.errorPages.find(intToString(this->statusCode)) == location.errorPages.end())
	{
		body = getDefaultErrorBody(this->statusCode);
	}
	else
	{
		body = getBodyFromFile(location.errorPages[intToString(this->statusCode)]);
	}
}

std::string Response::getBodyFromFile(std::string filePath)
{
	std::ifstream file(filePath.c_str());
	std::stringstream body;
	std::string fileName = filePath.substr(filePath.find_last_of("/") + 1);
	if (file.is_open())
	{
		std::string line;
		while (std::getline(file, line))
		{
			body << line << std::endl;
		}
		file.close();
		this->setMimeType(fileName);
		return body.str();
	}
	else
	{
		this->statusCode = 500;
		return http_error_500_page;
	}
}

void Response::setStatusLine()
{
	std::stringstream ss;
	ss << req.getVersion() << " " << statusCode << " " << getStatusMsg(statusCode) << LF;
	statusLine = ss.str();
}

void Response::createResponseStr(LocationBlock location)
{
	std::stringstream ss;
	setStatusLine();
	if (this->statusCode >= 400)
	{
		setErrorBody(location);
		setMimeType("html");
	}
	setHeaders(location);
	ss << statusLine << headers << body << LF;
	response = ss.str();
}

void Response::setMimeType(std::string const &fileName)
{
	if (fileName == "html")
	{
		mimeType = "text/html";
		return;
	}
	std::string extension = fileName.substr(fileName.find_last_of(".") + 1);
	if (extension == "html")
		mimeType = "text/html";
	else if (extension == "css")
		mimeType = "text/css";
	else if (extension == "js")
		mimeType = "text/javascript";
	else if (extension == "jpg")
		mimeType = "image/jpeg";
	else if (extension == "jpeg")
		mimeType = "image/jpeg";
	else if (extension == "png")
		mimeType = "image/png";
	else if (extension == "gif")
		mimeType = "image/gif";
	else if (extension == "bmp")
		mimeType = "image/bmp";
	else if (extension == "ico")
		mimeType = "image/x-icon";
	else if (extension == "svg")
		mimeType = "image/svg+xml";
	else if (extension == "mp3")
		mimeType = "audio/mpeg";
	else if (extension == "mp4")
		mimeType = "video/mp4";
	else if (extension == "webm")
		mimeType = "video/webm";
	else if (extension == "ogg")
		mimeType = "audio/ogg";
	else if (extension == "wav")
		mimeType = "audio/wav";
	else if (extension == "avi")
		mimeType = "video/x-msvideo";
	else if (extension == "mpeg")
		mimeType = "video/mpeg";
	else if (extension == "txt")
		mimeType = "text/plain";
	else
		mimeType = "application/octet-stream";
}

void Response::setHeaders(LocationBlock location)
{
	std::stringstream ss;
	ss << "Content-Type: " << mimeType << CRLF
	   << "Content-Length: " << body.size() << CRLF;
	if (location.redirection)
	{
		ss << "Location: " << location.redirects[statusCode] << CRLF;
	}
	ss << CRLF;
	headers = ss.str();
}

void Response::getBody(std::string uri, LocationBlock location)
{
	std::string path = setPath(location, uri);
	if (isDirectory(path))
	{
		DIR *directoryPtr = opendir(path.c_str());
		if (directoryPtr == NULL)
		{
			if (errno == ENOENT)
			{
				this->statusCode = 404;
				return;
			}
			else if (errno == EACCES)
			{
				this->statusCode = 403;
				return;
			}
			else
			{
				this->statusCode = 500;
				return;
			}
		}
		else
		{
			struct dirent *dir;
			while ((dir = readdir(directoryPtr)) != NULL)
			{
				std::string fileName = dir->d_name;
				if (fileName == "." || fileName == "..")
					continue;
				if (hasDefaultFile(path, location))
				{
					if (!isInIndex(fileName, location))
						continue;
					std::ifstream file(getFilePath(path, fileName).c_str());
					if (file.is_open())
					{
						std::stringstream body;
						std::string line;
						while (std::getline(file, line))
							body << line << std::endl;
						this->body = body.str();
						this->setMimeType(fileName);
						this->statusCode = 200;
						file.close();
					}
					else
						this->statusCode = 403;
					break;
				}
				else if (location.autoindex) // directory listing
				{
					this->body = generateDirectoryListingHTML(path, location.root);
					this->setMimeType("html");
					this->statusCode = 200;
					break;
				}
				else
				{
					this->statusCode = 403;
					break;
				}
			}
		}
		if (this->statusCode == 0)
			this->statusCode = 403;
		closedir(directoryPtr);
	}
	else if (isFile(path))
	{
		std::ifstream file(path.c_str());
		if (file.is_open())
		{
			std::string line;
			std::stringstream body;
			while (std::getline(file, line))
				body << line << std::endl;
			this->body = body.str();
			this->setMimeType(path);
			this->statusCode = 200;
			file.close();
		}
		else
			this->statusCode = 403;
	}
	else
		this->statusCode = 404;
}

std::string parseFileName(std::string body, std::string keyword)
{
	size_t pos = body.find(keyword);
	if (pos == std::string::npos)
	{
		std::cerr << "Could not find keyword " << keyword << " in body" << std::endl;
		return "";
	}
	pos += keyword.length();
	size_t endPos = body.find(CRLF, pos);
	if (endPos == std::string::npos)
	{
		std::cerr << "Could not find end of line in body" << std::endl;
		return "";
	}
	std::string fileName = body.substr(pos, endPos - 1 - pos);
	if (!fileName.empty() && fileName[fileName.size() - 1] == CR)
		fileName.erase(fileName.size() - 1);
	return fileName;
}

void Response::handleUploadFiles(LocationBlock &location, Request &req)
{
	std::string body = req.getBody();
	if (location.uploadLocation.empty())
	{
		this->statusCode = 404;
		return;
	}
	DIR *dir = opendir(location.uploadLocation.c_str());
	if (dir == NULL)
	{
		if (errno == ENOENT)
		{
			this->statusCode = 404;
			return;
		}
		else if (errno == EACCES)
		{
			this->statusCode = 403;
			return;
		}
		else
		{
			this->statusCode = 500;
			return;
		}
	}
	else
	{
		std::string fileName = "";
		std::string contentType = getContentType(req.getHeaders()["Content-Type"]);
		if (contentType == "multipart/form-data")
			fileName = parseFileName(body, "filename=\"");
		if (!fileName.empty())
		{
			if (fileName[0] == '/')
				fileName = fileName.substr(1);
		}
		else
		{
			this->statusCode = 400;
			closedir(dir);
			return;
		}
		if (checkIfFileExists(location.uploadLocation, fileName) == 0)
		{
			this->statusCode = 409;
			closedir(dir);
			return;
		}
		if (chdir(location.uploadLocation.c_str()) == -1)
		{
			this->statusCode = 403;
			closedir(dir);
			return;
		}
		std::ofstream file(fileName.c_str());
		if (!file.is_open())
		{
			this->statusCode = 403;
			closedir(dir);
			changeDirBack(location.uploadLocation);
			return;
		}
		std::string fileContent = getFileContent(req.getBody(), req);
		file << fileContent;
		file.close();
		this->statusCode = 201;
		changeDirBack(location.uploadLocation);
	}
	closedir(dir);
}
void Response::handleGetRequest(LocationBlock location)
{
	if (needsCGI(location, req))
	{
		handleCGI(location, req, *this);
		return;
	}
	getBody(req.getUri(), location);
	return;
}

void Response::handlePostRequest(LocationBlock location)
{
	if (needsCGI(location, req))
	{
		handleCGI(location, req, *this);
		return;
	}
	handleUploadFiles(location, req);
	body = uploadSuccess;
	setMimeType("html");
	return;
}

void Response::handleDeleteRequest(LocationBlock location)
{
	if (needsCGI(location, req))
	{
		handleCGI(location, req, *this);
		return;
	}
	std::string uri = req.getUri();
	std::string filePath = location.root + uri;
	struct stat fileStat;
	if (stat(filePath.c_str(), &fileStat) == -1)
	{
		this->statusCode = 404;
		setMimeType("html");
		return;
	}
	else
	{
		if (remove(filePath.c_str()) != 0)
		{
			this->statusCode = 500;
			return;
		}
		body = deleteSuccess;
		setMimeType("html");
		this->statusCode = 200;
		return;
	}
}

void Response::methodCheck(LocationBlock location)
{
	if (req.getMethod() == UNKNOWN)
	{
		this->statusCode = 405;
		return;
	}
	if (location.methods.empty() || std::find(location.methods.begin(), location.methods.end(), req.getMethod()) == location.methods.end())
	{
		this->statusCode = 405;
	}
}

void Response::bodySizeCheck(Configuration &config, LocationBlock &location)
{
	int maxBodySize = config.getBodySize(location.clientMaxBodySize);
	if (maxBodySize == 0)
		return;
	std::string contentLength = req.getHeaders()["Content-Length"];
	if (stringToInt(contentLength) > maxBodySize)
	{
		if (this->statusCode == 0)
			this->statusCode = 413;
	}
}

std::string Response::handleRedirection(LocationBlock &location)
{
	std::stringstream ss;
	body = "";
	setMimeType("html");
	this->statusCode = 307;
	setStatusLine();
	setHeaders(location);
	createResponseStr(location);
	return response;
}
void printRequest(Request &req)
{
	std::cerr << std::string(21, '*') << std::endl;
	std::cerr << "Method: " << req.getMethod() << std::endl;
	std::cerr << "URI: " << req.getUri() << std::endl;
	std::cerr << "Version: " << req.getVersion() << std::endl;
	std::cerr << "Headers: " << std::endl;
	std::map<std::string, std::string> headers = req.getHeaders();
	for (std::map<std::string, std::string>::iterator it = headers.begin(); it != headers.end(); ++it)
	{
		std::cerr << it->first << ":" << it->second << std::endl;
	}
	std::cerr << "Body: " << std::endl;
	std::cerr << req.getBody() << std::endl;
	std::cerr << std::endl;
	std::cerr << std::string(21, '*') << std::endl;
}
std::string Response::getResponse(Configuration &config)
{
	LocationBlock location;
	initLocationBlock(location);
	if (serverBlockExists(config, this->req))
	{
		location = getLocationFromServer(config, this->req);
		methodCheck(location);
		bodySizeCheck(config, location);
		if (location.redirection == true)
			return (handleRedirection(location));
		if (this->statusCode == 0)
		{
			switch (req.getMethod())
			{
			case GET:
				handleGetRequest(location);
				break;
			case POST:
				handlePostRequest(location);
				break;
			case DELETE:
				handleDeleteRequest(location);
				break;
			}
		}
	}
	createResponseStr(location);
	return response;
}

LocationBlock Response::getLocationFromServer(Configuration &config, Request &req)
{
	ServerBlock serverBlock;
	LocationBlock location;
	std::string hostName = req.getHost();
	int port = req.getPort();
	int serverBlockCount = serverBlocksCount(config, hostName, port);
	if (serverBlockCount > 1 && matchExists(config, hostName, port))
		serverBlock = getMatchingServerBlock(config, hostName, port);
	else
		serverBlock = getDefaultServerBlock(config, hostName, port);
	std::string uri = req.getUri();
	if (locationBlockExists(serverBlock, uri))
		location = getMatchingLocationBlock(serverBlock, uri);
	else if (serverBlock.locationBlocks.size() == 0)
		location = serverBlock.locationBlocks[0];
	else
		location = getMatchingLocationBlock(serverBlock, "/");
	return location;
}

void Response::clearResponse(void)
{
	this->statusCode = 0;
	this->headers.clear();
	this->body.clear();
	this->response.clear();
	this->mimeType.clear();
}
