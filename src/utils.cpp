#include "Webserv2.hpp"
#include "Response.hpp"

std::string intToString(int value)
{
	std::stringstream ss;
	ss << value;
	return ss.str();
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
		if (it->host == host && it->port == port)
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
		if (it->host == host && it->port == port)
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
		if (it->host == host && it->port == port)
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
			if (it->host == host && it->port == port && *it2 == host)
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
			if (it->host == host && it->port == port && *it2 == host)
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
	return *location;
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

bool hasDefaultFile(const std::string &directoryPath, std::string fileName, LocationBlock location)
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

std::string getFilePath(std::string path, std::string uri, std::string fileName)
{
	std::string filePath;
	if (path[path.size() - 1] != '/')
		path += "/";
	if (uri != "/")
		filePath = path + fileName;
	else
		filePath = path + "index.html";
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