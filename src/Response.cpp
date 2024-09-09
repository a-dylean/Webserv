#include "Response.hpp"

Response::Response() {};

Response::Response(Request &req) : req(req) {};

Response::~Response() {};

std::string Response::getStatusMsg(int code)
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
	case 501:
		return "Not Implemented";
	case 505:
		return "HTTP Version Not Supported";
	}
	return "Internal Server Error";
}
void Response::setBody(std::string const &body)
{
	this->body = body;
}
void Response::setStatusLine()
{
	std::stringstream ss;
	ss << req.getVersion() << " " << statusCode << " " << getStatusMsg(statusCode) << "\n";
	statusLine = ss.str();
}

void Response::createResponseStr()
{
	std::stringstream ss;
	setStatusLine();
	setHeaders();
	ss << statusLine << headers << body << "\n";
	response = ss.str();
}

void Response::setHeaders()
{
	std::stringstream ss;
	ss << "Content-Type: text/html\r\n"
	   << "Content-Length: " << body.size() << "\r\n\r\n";
	headers = ss.str();
}

void Response::handleGetRequest(Configuration &config)
{
	processServerBlock(config, this->req);
	createResponseStr();
}

void Response::handlePostRequest(Configuration &config)
{
	return;
}

void Response::handleDeleteRequest(Configuration &config)
{
	return;
}
std::string Response::getResponse(Configuration &config)
{
	switch (req.getMethod())
	{
	case GET:
		handleGetRequest(config);
		break;
	case POST:
		handlePostRequest(config);
		break;
	case DELETE:
		handleDeleteRequest(config);
		// case UNKNOWN:
		// 	handleUnknownRequest();
		break;
	}
	return response;
}

std::string Response::getPath(std::vector<ServerBlock>::iterator it, std::string uri)
{
	std::vector<LocationBlock> locationBlocks = it->locationBlocks;
	for (std::vector<LocationBlock>::iterator it = locationBlocks.begin(); it != locationBlocks.end(); it++)
	{
		std::cout << "PATH: " << it->path << std::endl;
		std::cout << "URI: " << it->root << std::endl;
		if (it->path == uri)
		{
			return it->root;
		}
	}
	return "";
}

bool Response::locationBlockExists(std::vector<ServerBlock>::iterator it, std::string uri)
{
	std::vector<LocationBlock> locationBlocks = it->locationBlocks;
	for (std::vector<LocationBlock>::iterator it = locationBlocks.begin(); it != locationBlocks.end(); it++)
	{
		if (it->path == uri)
		{
			return true;
		}
	}
	return false;
}

LocationBlock Response::getLocationBlock(std::vector<ServerBlock>::iterator it, std::string uri)
{
	LocationBlock *location = NULL;
	std::vector<LocationBlock> locationBlocks = it->locationBlocks;
	for (std::vector<LocationBlock>::iterator it = locationBlocks.begin(); it != locationBlocks.end(); it++)
	{
		if (it->path == uri)
		{
			return *it;
		}
	}
	return *location;
}

void Response::setErrorBody(std::ifstream &file, std::stringstream &body)
{
	std::string line;
	while (std::getline(file, line))
	{
		body << line << std::endl;
	}
	file.close();
	this->body = body.str();
}

void Response::handleErrorPage(std::vector<ServerBlock>::iterator it)
{
	// bool errorPageExists = false;

	// if (it->errorPages.size() != 0)
	// {
	// 	for (std::map<std::string, std::string>::iterator it2 = it->errorPages.begin(); it2 != it->errorPages.end(); it2++)
	// 	{
	// 		if (it2->first == std::to_string(this->statusCode))
	// 		{
	// 			std::string filePath = it->root + "/" + it2->second;
	// 			std::ifstream file(filePath.c_str());
	// 			std::stringstream body;
	// 			if (file.is_open())
	// 				setErrorBody(file, body);
	// 			else
	// 			{
	// 				this->statusCode = 500;
	// 				std::string filePath = it->root + "/error500.html";
	// 				std::ifstream file(filePath.c_str());
	// 				if (file.is_open())
	// 					setErrorBody(file, body);
	// 			}
	// 		}
	// 	}
	// 	errorPageExists = true;
	// }
	// if (errorPageExists == false)
	// {
	// 	//default error pages
	// }
	std::ifstream file("custom404.html");
	std::stringstream body;
	if (file.is_open())
	{
		std::string line;
		while (std::getline(file, line))
		{
			body << line << std::endl;
		}
		file.close();
		this->body = body.str();
	}
}


void Response::handleRoot(std::string configPath, std::string requestUri)
{
	DIR *directoryPtr = opendir(configPath.c_str());
	if (directoryPtr == NULL)
	{
		if (errno == ENOENT)
		{
			std::cout << "Directory does not exist" << std::endl;
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
			std::string filePath = configPath + "/" + fileName;
			if (requestUri == "/")
			{
				filePath = configPath + "/" + "index.html";
			}
			if (requestUri == "/" || requestUri == "/" + fileName)
			{
				std::ifstream file(filePath.c_str());
				std::stringstream body;
				if (file.is_open())
				{
					std::string line;
					while (std::getline(file, line))
					{
						body << line << std::endl;
					}
					file.close();
					this->body = body.str();
					this->statusCode = 200;
				}
				else
				{
					this->statusCode = 403;
				}
			}
		}
		if (this->statusCode == 0)
		{
			std::cout << "404 - not found" << std::endl;
			this->statusCode = 404;
		}
		closedir(directoryPtr);
	}
};

void Response::processServerBlock(Configuration &config, Request &req)
{
	std::vector<ServerBlock>::iterator it;
	std::vector<ServerBlock> serverBlocks = config.getServerBlocks();
	std::string host = req.getHeaders()["Host"];
	size_t pos = host.find(':');
	std::string hostName = host.substr(0, pos);
	std::string portValue = host.substr(pos + 1);
	int port;
	std::stringstream ss(portValue);
	ss >> port;
	for (std::vector<ServerBlock>::iterator it = serverBlocks.begin(); it != serverBlocks.end(); it++)
	{
		if (it->host == hostName && it->port == port)
		{
			std::string uri = req.getUri();
			if (locationBlockExists(it, uri))
			{
				LocationBlock location = getLocationBlock(it, uri);
				if (location.path.size() == 0)
				{
					std::cout << "Path not found" << std::endl;
					statusCode = 404;
					return;
				}
				int maxBodySize = config.getClientMaxBodySize(location.clientMaxBodySize);
				if (req.getBody().size() > maxBodySize)
				{
					std::cout << "Payload too large" << std::endl;
					statusCode = 413;
					return;
				}
				handleRoot(location.root, uri);
			}	
			else 
				handleRoot(it->root, uri);
			if (this->statusCode != 200)
				handleErrorPage(it);
		}
	}
}