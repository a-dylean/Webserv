#include "Response.hpp"
#include "Webserv.hpp"

Response::Response() : statusCode(0), fdToClose(false) {};

Response::Response(Request &req) : req(req), statusCode(0), fdToClose(false) {};

Response::~Response() {};

void Response::setStatusCode(int code)
{
	this->statusCode = code;
}

void Response::setBody(std::string const &body)
{
	this->body = body;
}

// change this to setErrorBody ?
void Response::setErrorBody(LocationBlock location)
{
	if (location.errorPages.empty())
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
		setErrorBody(location);
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
	// add text/html as a default mime type (for default errors) ?
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
	// std::cout << "Path: " << path << std::endl;
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
				if (hasDefaultFile(path, fileName, location))
				{
					if (!isInIndex(fileName, location))
						continue;
					std::ifstream file(getFilePath(path, uri, fileName).c_str());
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
			this->statusCode = 404;
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
		std::cerr << "Error: Could not find keyword " << keyword << " in body." << std::endl;
        return "";
	}
    pos += keyword.length();
	size_t endPos = body.find("\r\n", pos);
	if (endPos == std::string::npos)
	{
		std::cerr << "Error: Could not find end of line in body." << std::endl;
		return "";
	}
	std::string fileName = body.substr(pos, endPos - 1 - pos);
	if (!fileName.empty() && fileName[fileName.size() - 1] == '\r')
		fileName.erase(fileName.size() - 1);
	return fileName;
}

// int countSlashes(std::string path)
// {
// 	int count = 0;
// 	for (size_t i = 0; i < path.size(); i++)
// 	{
// 		if (path[i] == '/')
// 			count++;
// 	}
// 	if (path[path.size() - 1] == '/')
// 		count--;
// 	return count;
// }

// void changeDirBack(std::string path)
// {
// 	int slashes = countSlashes(path);
// 	for (int i = 0; i < slashes; i++)
// 	{
// 		if (chdir("../") == -1)
// 		{
// 			std::cerr << "Error: Could not change directory back." << std::endl;
// 			return;
// 		}
// 	}
// }

void Response::handleUploadFiles(Configuration &config, LocationBlock &location, Request &req)
{
	(void)config;
	//on va verifier s'il existe bien une upload location dans le location block si non erreur (trouver quel numero d'erreur)
	std::string body = req.getBody();
	if (location.uploadLocation.empty())
	{
		this->statusCode = 405; //check si bon code
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
		// std::cout << "contentType: " << contentType << std::endl;
		if (contentType == "multipart/form-data")
		{
			fileName = parseFileName(body, "filename=\"");
			// std::cout << "fileName: " << fileName << std::endl;
		}
		if (fileName != "")
		{
			if (fileName[0] == '/')
				fileName = fileName.substr(1);
		}
		if (fileName == "")
		{
			fileName = setDefaultFileName(location.uploadLocation);
			if (fileName == "error")
			{
				this->statusCode = 500;
				return;
			}
		}
		if (checkIfFileExists(location.uploadLocation, uploadNb, fileName) == 0)
		{
			std::string fileCopy = setFileCopyName(fileName);
			fileName = fileCopy;
		}
		if (chdir(location.uploadLocation.c_str()) == -1)
		{
			this->statusCode = 500;
			return;
		}
		std::ofstream file(fileName.c_str());
		if (!file.is_open()) 
		{
			//remplacer par throw exception? 
			std::cerr << "Error: Could not open file " << fileName << " for writing." << std::endl;
			this->statusCode = 500;
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

void Response::handleGetRequest(Configuration &config, LocationBlock location)
{
	std::string uri = req.getUri();
	if (!location.cgiExtensions.empty())
	{
		for (std::vector<std::string>::iterator it = location.cgiExtensions.begin(); it != location.cgiExtensions.end(); it++)
		{
			if (uri.find(*it) != std::string::npos)
			{
				handleCGI(config, location, req, *this);
				return;
			}
		}
		struct stat fileStat;
		if (stat((location.root + uri).c_str(), &fileStat) == -1)
		{
			this->statusCode = 404;
			setMimeType("html");
			return;
		}
	}
	getBody(req.getUri(), location);
	return;
}

void Response::handlePostRequest(Configuration &config, LocationBlock location)
{
	if (location.cgiParams.empty())
	{
		handleUploadFiles(config, location, req);
		// body = "<div style=\"display: flex; justify-content: center; align-items: center; height: 100vh; color: green; font-weight: bold;\">Upload was successful!</div>";
		body = "<html><head><title>Upload Successful</title></head><body><div style=\"display: flex; justify-content: center; align-items: center; height: 100vh; color: green; font-weight: bold;\"><h1>Upload was successful!</h1></div></body></html>";
		setMimeType("html");
		return;
	}
	handleCGI(config, location, req, *this);
	return;
}

void Response::handleDeleteRequest(Configuration &config, LocationBlock location)
{
	(void)config;
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
			setMimeType("html");
			return;
		}
		body = "<html><head><title>File successfully deleted!</title></head><body><div><h1>Delete operation was successful!</h1></div></body></html>";
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
		setMimeType("html");
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
	// compare with content length header!!!
	if ((int)req.getBody().size() > maxBodySize)
	{
		if (this->statusCode == 0)
			this->statusCode = 413;
	}
}

std::string Response::handleRedirection(Configuration &config, LocationBlock &location)
{
	(void)config;
	std::stringstream ss;
	body = "";
	setMimeType("html");
	statusCode = 307;
	setStatusLine();
	setHeaders(location);
	createResponseStr(location);
	return response;
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
			return (handleRedirection(config, location));
		if (this->statusCode == 0)
		{
			switch (req.getMethod())
			{
			case GET:
				handleGetRequest(config, location);
				break;
			case POST:
				handlePostRequest(config, location);
				break;
			case DELETE:
				handleDeleteRequest(config, location);
				break;
			}
		}
	}
	createResponseStr(location);
	return response;
}

// void Response::handleRoot(std::string configPath, std::string requestUri)
// {
// 	DIR *directoryPtr = opendir(configPath.c_str());
// 	if (directoryPtr == NULL)
// 	{
// 		if (errno == ENOENT)
// 		{
// 			this->statusCode = 404;
// 			return;
// 		}
// 		else if (errno == EACCES)
// 		{
// 			this->statusCode = 403;
// 			return;
// 		}
// 		else
// 		{
// 			this->statusCode = 500;
// 			return;
// 		}
// 	}
// 	else
// 	{
// 		struct dirent *dir;
// 		while ((dir = readdir(directoryPtr)) != NULL)
// 		{
// 			std::string fileName = dir->d_name;
// 			// std::cout << "fileName: " << fileName << std::endl;
// 			if (fileName == "." || fileName == "..")
// 				continue;
// 			std::string filePath = configPath + "/" + fileName;
// 			if (requestUri == "/")
// 			{
// 				filePath = configPath + "/" + "index.html";
// 			}
// 			if (requestUri == "/" || requestUri == "/" + fileName)
// 			{
// 				std::ifstream file(filePath.c_str());
// 				std::stringstream body;
// 				if (file.is_open())
// 				{
// 					std::string line;
// 					while (std::getline(file, line))
// 					{
// 						body << line << std::endl;
// 					}
// 					file.close();
// 					this->body = body.str();
// 					if (requestUri == "/")
// 					{
// 						this->setMimeType("index.html");
// 					}
// 					else
// 					{
// 						this->setMimeType(fileName);
// 					}
// 					this->statusCode = 200;
// 					break;
// 				}
// 				else
// 				{
// 					this->statusCode = 403;
// 					break;
// 				}
// 			}
// 		}
// 		if (this->statusCode == 0)
// 		{
// 			this->statusCode = 404;
// 		}
// 		closedir(directoryPtr);
// 	}
// };

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