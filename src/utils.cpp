#include "Webserv.hpp"
#include "Response.hpp"

std::string intToString(int value)
{
	std::stringstream ss;
	ss << value;
	return ss.str();
}

int stringToInt(const std::string &str)
{
	std::stringstream ss(str);
	int value;
	ss >> value;
	return value;
}

bool isDirectory(const std::string &path)
{
	struct stat s;
	if (stat(path.c_str(), &s) == 0)
	{
		if (s.st_mode & S_IFDIR)
			return true;
	}
	return false;
}

bool isFile(const std::string &path)
{
	struct stat s;
	if (stat(path.c_str(), &s) == 0)
	{
		if (s.st_mode & S_IFREG)
			return true;
	}
	return false;
}
bool serverBlockExists(Configuration &config, Request &req)
{
	std::vector<ServerBlock> serverBlocks = config.getServerBlocks();
	std::string host = req.getHost();
	int port = req.getPort();
	for (std::vector<ServerBlock>::iterator it = serverBlocks.begin(); it != serverBlocks.end(); it++)
	{
		if (it->hostPort.first == host && it->hostPort.second == port)
			return true;
	}
	return false;
}
int serverBlocksCount(Configuration &config, std::string host, int port)
{
	size_t count = 0;
	std::vector<ServerBlock> serverBlocks = config.getServerBlocks();
	for (std::vector<ServerBlock>::iterator it = serverBlocks.begin(); it != serverBlocks.end(); it++)
	{
		if (it->hostPort.first == host && it->hostPort.second == port)
			count++;
	}
	return count;
}

ServerBlock getDefaultServerBlock(Configuration &config, std::string host, int port)
{
	ServerBlock *serverBlock = NULL;
	std::vector<ServerBlock> serverBlocks = config.getServerBlocks();
	for (std::vector<ServerBlock>::iterator it = serverBlocks.begin(); it != serverBlocks.end(); it++)
	{
		if (it->hostPort.first == host && it->hostPort.second == port)
			return *it;
	}
	return *serverBlock;
}

bool matchExists(Configuration &config, std::string host, int port)
{
	std::vector<ServerBlock> serverBlocks = config.getServerBlocks();
	for (std::vector<ServerBlock>::iterator it = serverBlocks.begin(); it != serverBlocks.end(); ++it)
	{
		for (std::vector<std::string>::iterator it2 = it->serverNames.begin(); it2 != it->serverNames.end(); ++it2)
		{
			if (it->hostPort.first == host && it->hostPort.second == port && *it2 == host)
				return true;
		}
	}
	return false;
}

ServerBlock getMatchingServerBlock(Configuration &config, std::string host, int port)
{
	ServerBlock *serverBlock = NULL;
	std::vector<ServerBlock> serverBlocks = config.getServerBlocks();
	for (std::vector<ServerBlock>::iterator it = serverBlocks.begin(); it != serverBlocks.end(); it++)
	{
		for (std::vector<std::string>::iterator it2 = it->serverNames.begin(); it2 != it->serverNames.end(); ++it2)
		{
			if (it->hostPort.first == host && it->hostPort.second == port && *it2 == host)
				return *it;
		}
	}
	return *serverBlock;
}

bool locationBlockExists(ServerBlock serverBlock, std::string uri) {
    std::vector<LocationBlock> locationBlocks = serverBlock.locationBlocks;
    for (std::vector<LocationBlock>::iterator it = locationBlocks.begin(); it != locationBlocks.end(); it++) {
        if (it->path == uri) {
            return true;
        }
    }

    // If no match is found, remove the last part after the last '/' and check again
    std::size_t pos = uri.find_last_of('/');
    if (pos != std::string::npos && pos != 0) {
        return locationBlockExists(serverBlock, uri.substr(0, pos));
    }

    return false;
}

LocationBlock getMatchingLocationBlock(ServerBlock serverBlock, std::string uri)
{	
	LocationBlock *location = NULL;
    std::vector<LocationBlock> locationBlocks = serverBlock.locationBlocks;
    for (std::vector<LocationBlock>::iterator it = locationBlocks.begin(); it != locationBlocks.end(); it++) {
        if (it->path == uri) {
            return *it;
        }
    }

    // If no match is found, remove the last part after the last '/' and check again
    std::size_t pos = uri.find_last_of('/');
    if (pos != std::string::npos && pos != 0) {
        return getMatchingLocationBlock(serverBlock, uri.substr(0, pos));
    }
	return *location;// TODO : maybe replace with throw and catch it in the main without stopping the server
}

std::string getDefaultErrorBody(int statusCode)
{
	for (std::map<int, std::string>::const_iterator it = http_error_pages.begin(); it != http_error_pages.end(); it++)
	{
		if (it->first == statusCode)
			return it->second;
	}
	return http_error_500_page;
}
bool isInIndex(std::string fileName, LocationBlock location)
{
	if (fileName == "index.html")
		return true;
	for (std::vector<std::string>::iterator it = location.indexes.begin(); it != location.indexes.end(); it++)
	{
		if (fileName == *it)
			return true;
	}
	return false;
}

bool hasDefaultFile(const std::string &directoryPath, LocationBlock location)
{
	DIR *directoryPtr = opendir(directoryPath.c_str());
	struct dirent *dir;
	if (directoryPtr == NULL)
		return false;
	while ((dir = readdir(directoryPtr)) != NULL)
	{
		std::string fileName = dir->d_name;
		if (fileName == "." || fileName == "..")
			continue;
		if (fileName == "index.html")
		{
			closedir(directoryPtr);
			return true;
		}
		for (size_t i = 0; i < location.indexes.size(); ++i)
		{
			if (fileName == location.indexes[i])
			{
				closedir(directoryPtr);
				return true;
			}
		}
	}
	closedir(directoryPtr);
	return false;
}

std::string getFilePath(std::string path, std::string fileName)
{
	std::string filePath;
	if (path[path.size() - 1] != '/')
		path += "/";
	// if (uri != "/")
		filePath = path + fileName;
	// else
	// 	filePath = path + "index.html";
	return filePath;
}

std::string generateDirectoryListingHTML(const std::string &directoryPath, const std::string &rootPath)
{
	DIR *directoryPtr = opendir(directoryPath.c_str());
	if (directoryPtr == NULL)
	{
		std::cerr << "opendir failed for path: " << directoryPath << " with error: " << strerror(errno) << std::endl;
		return "<html><body><h1>Failed to open directory</h1></body></html>";
	}

	std::stringstream html;
	html << "<html><head><title>Directory Listing</title></head><body>";
	html << "<h1>Index of " << directoryPath.substr(rootPath.size()) << "</h1>";
	html << "<ul>";

	struct dirent *dir;
	while ((dir = readdir(directoryPtr)) != NULL)
	{
		std::string fileName = dir->d_name;
		if (fileName == ".")
			continue;
		std::string fullPath;
		std::size_t pos = directoryPath.find(rootPath);
		std::string updatedDirectoryPath;
		if (pos != std::string::npos)
		{
			updatedDirectoryPath = directoryPath.substr(pos + rootPath.size());
		}
		if (directoryPath[directoryPath.size() - 1] != '/')
			fullPath = updatedDirectoryPath + "/" + fileName;
		else
			fullPath = updatedDirectoryPath + fileName;
		struct stat statbuf;
		if (stat(fullPath.c_str(), &statbuf) == 0 && S_ISDIR(statbuf.st_mode))
		{
			html << "<li><a href=\"" << fullPath << "/index.html\">" << fileName << "/</a></li>";
		}
		else
		{
			html << "<li><a href=\"" << fullPath << "\">" << fileName << "</a></li>";
		}
	}
	html << "</ul>";
	html << "</body></html>";

	closedir(directoryPtr);
	return html.str();
}

std::string setPath(LocationBlock location, std::string uri)
{
	std::string path;
	if (location.alias != "")
	{
		path = location.root + location.alias;
		if (uri != "/")
			path += uri.substr(location.path.size());
	}
	else
	{
		if (uri == "/")
		{
			path = location.root;
		}
		else
		{
			path = location.root + uri;
		}
	}
	return path;
}

int checkIfFileExists(const std::string &dirPath, std::string targetFileName) 
{
    DIR *dir = opendir(dirPath.c_str());
    if (dir == NULL) 
	{
        std::cerr << "Error: Could not open directory " << dirPath << std::endl;
        return -1;
    }
    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) 
	{
        if (entry->d_name == targetFileName) 
		{
            closedir(dir);
            return 0; //true
        }
    }
    closedir(dir);
    return 1; //false
}

static std::string getContentBetweenBoundaries(std::string body, size_t firstBoundaryPos, size_t secondBoundaryPos)
{
	size_t contentStartPos = firstBoundaryPos + 2; // +2 to skip \r\n
	size_t contentEndPos = secondBoundaryPos - 2; // -2 to skip \r\n
	std::string content = body.substr(contentStartPos, contentEndPos - contentStartPos);
	return content;
}

std::string getContentType(const std::string &contentType)
{
    size_t pos = contentType.find(";");
    if (pos == std::string::npos)
        return "";

    std::string type = contentType.substr(0, pos);
    return type;
}

int getNbBoundaries(std::string body, std::string boundary)
{
	int count = 0;
	size_t pos = 0;
	while ((pos = body.find(boundary, pos)) != std::string::npos)
	{
		count++;
		pos += boundary.length();
	}
	return count;
}

std::string getFileBody(std::string body, std::string &boundary)
{
	std::string fileBody;
	int nbBoundaries = getNbBoundaries(body, boundary);
	if (nbBoundaries < 2)
		return "";
	
	size_t firstBoundaryPos = body.find(boundary);
	if (firstBoundaryPos == std::string::npos)
		return "";
	size_t secondBoundaryPos = body.find(boundary, firstBoundaryPos + boundary.length());
	if (secondBoundaryPos == std::string::npos)
		return "";
	fileBody = getContentBetweenBoundaries(body, firstBoundaryPos, secondBoundaryPos);
	return fileBody;
}

std::string getFileContent(std::string body, Request &req)
{
	std::string contentType = getContentType(req.getHeaders()["Content-Type"]);
	if (contentType != "multipart/form-data")
		return body;

	std::string boundary = body.substr(0, body.find(CRLF));
	if (boundary.empty())
		return "";
	std::string fileBody = getFileBody(body, boundary);
	if (fileBody.empty())
		return "";
	size_t contentStartPos = fileBody.find("\r\n\r\n") + 4;
	if (contentStartPos == std::string::npos)
		return "";
	size_t contentEndPos = fileBody.find(CRLF) - 2;
	std::string fileContent = fileBody.substr(contentStartPos, contentEndPos - contentStartPos);
	if (fileBody.empty())
		return "";
	return fileContent;
}

static int countSlashes(std::string path)
{
	int count = 0;
	for (size_t i = 0; i < path.size(); i++)
	{
		if (path[i] == '/')
			count++;
	}
	if (path[path.size() - 1] == '/')
		count--;
	return count;
}

void changeDirBack(std::string path)
{
	int slashes = countSlashes(path);
	for (int i = 0; i < slashes; i++)
	{
		if (chdir("../") == -1)
		{
			std::cerr << "Error: Could not change directory back." << std::endl;
			return;
		}
	}
}