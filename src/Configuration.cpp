#include "../include/Configuration.hpp"
#include "Configuration.hpp"

// TODO :
// add default location block even if we have some location blocks
// need a function to comapre location blocks and if we don't have a default one, add it

static void printSplit(std::vector<std::string> split)
{
	std::cout << "split size : " << split.size() << std::endl;
	std::cout << "split : ";
	for (std::vector<std::string>::iterator it = split.begin(); it != split.end(); ++it)
		std::cout << *it << " ";
	std::cout << std::endl;
}

static bool isValidHost(std::string const &host)
{
	// size_t	dot = 0;

	if (host.empty())
		return false;
	// if (host == "localhost")
	// 	return true;
	// for (size_t i = 0; i < host.size(); i++)
	// {
	// 	if (host.at(i) == '.')
	// 		dot++;
	// }
	// if (dot != 3 || host.find_first_of("0123456789.") == std::string::npos)
	// 	return false;
	// if (host.find_first_not_of("0123456789.") != std::string::npos)
	// 	return false;
	// if (host.find_first_of(".") == 0 || host.find_last_of(".") == host.size() - 1)
	// 	return false;
	// for (size_t i = 0; i < host.size(); i++)
	// {
	// 	if (host.at(i) == '.')
	// 	{
	// 		if (host.at(i + 1) == '.')
	// 			return false;
	// 	}
	// }
	return true;
}

static std::vector<std::string> stringSplit(std::string const &line, char delim)
{
	std::vector<std::string> res;
	std::string word;
	std::stringstream ss(line);

	while (std::getline(ss, word, delim))
	{
		if (!word.empty())
			res.push_back(word);
	}
	return res;
}

static void initServerBlock(ServerBlock &serverBlock)
{
	serverBlock.hostPort.first = "localhost";
	serverBlock.hostPort.second = 8080;
	serverBlock.serverNames.clear();
	serverBlock.root = "./www";
	serverBlock.clientMaxBodySize.value = "0";
	serverBlock.clientMaxBodySize.unit = "G";
	serverBlock.bodySize = 0;
	serverBlock.autoindex = false;
	serverBlock.indexes.clear();
	serverBlock.errorPages.clear();
	serverBlock.redirects.clear();
	serverBlock.redirection = false;
	serverBlock.cgiParams.clear();
	serverBlock.methods.clear();
	serverBlock.locationBlocks.clear();
	serverBlock.listenHasBeenSet = false;
}

void initLocationBlock(LocationBlock &locationBlock)
{
	locationBlock.path = "/";
	locationBlock.alias = "";
	locationBlock.root = "";
	locationBlock.clientMaxBodySize.value = "";
	locationBlock.clientMaxBodySize.unit = "";
	locationBlock.bodySize = 0;
	locationBlock.autoindex = false;
	locationBlock.autoindexDone = false;
	locationBlock.pathInfo = false;
	locationBlock.indexes.clear();
	locationBlock.errorPages.clear();
	locationBlock.uploadLocation = "./www/upload";
	locationBlock.cgiParams.clear();
	locationBlock.redirects.clear();
	locationBlock.redirection = false;
	locationBlock.cgiParams.clear();
	locationBlock.methods.clear();
}

static void pushServerBlock(std::vector<ServerBlock> &serverBlocks, ServerBlock &serverBlock)
{
	if (serverBlock.serverNames.empty())
		serverBlock.serverNames.push_back("webserv");
	if (serverBlock.locationBlocks.empty())
	{
		LocationBlock locationBlock;

		initLocationBlock(locationBlock);
		locationBlock.root = serverBlock.root;
		locationBlock.errorPages = serverBlock.errorPages;
		locationBlock.clientMaxBodySize.value = serverBlock.clientMaxBodySize.value;
		locationBlock.clientMaxBodySize.unit = serverBlock.clientMaxBodySize.unit;
		serverBlock.locationBlocks.push_back(locationBlock);
	}
	serverBlocks.push_back(serverBlock);
}

static BodySize createBodySize(std::string const &value)
{
	BodySize bodySize;
	std::string unit;
	std::string number;

	if (value.empty() || value.at(0) == '\0' || value == "0")
	{
		bodySize.value = "0";
		bodySize.unit = 'K';
		return bodySize;
	}
	number = value.substr(0, value.find_first_not_of("0123456789"));
	if (number.empty() || number.at(0) == '\0')
		throw std::runtime_error("client_max_body_size no number found");
	unit = value.substr(value.find_first_not_of("0123456789"));
	if (unit.size() != 1)
		throw std::runtime_error("client_max_body_size invalid unit size");
	if (unit.at(0) == 'K' || unit.at(0) == 'k')
		unit = "K";
	else if (unit.at(0) == 'M' || unit.at(0) == 'm')
		unit = "M";
	else if (unit.at(0) == 'G' || unit.at(0) == 'g')
		unit = "G";
	else
		throw std::runtime_error("client_max_body_size invalid unit (k/K, m/M, g/G)");
	bodySize.value = number;
	bodySize.unit = unit;
	return bodySize;
}

static bool isHttpCode(std::string const &code)
{
	if (code.size() != 3)
		return false;
	if (code.find_first_not_of("0123456789") != std::string::npos)
		return false;
	return true;
}

static void setClientMaxBodySize(BodySize &clientMaxBodySize, int &bodySize, std::vector<std::string> split)
{
	std::stringstream ss;

	if (split.size() != 1)
		throw std::runtime_error("client max body size usage : {SIZE}[UNIT] (42M or 300K or 9G)");
	clientMaxBodySize = createBodySize(split[0]);
	ss << clientMaxBodySize.value;
	ss >> bodySize;
	if (clientMaxBodySize.unit == "K")
		bodySize *= 1024;
	else if (clientMaxBodySize.unit == "M")
		bodySize *= 1024 * 1024;
	else if (clientMaxBodySize.unit == "G")
		bodySize *= 1024 * 1024 * 1024;
}

static void setAllowedMethods(std::vector<http_method> &methods, std::vector<std::string> split)
{
	std::vector<std::string> methodsSplit;

	if (split.size() < 1)
	{
		methods.push_back(GET);
		methods.push_back(POST);
		methods.push_back(DELETE);
		return;
	}
	for (std::vector<std::string>::iterator it = split.begin(); it != split.end(); ++it)
	{
		methodsSplit = stringSplit(*it, '|');
		for (std::vector<std::string>::iterator it2 = methodsSplit.begin(); it2 != methodsSplit.end(); ++it2)
		{
			if (*it2 == "GET")
			{
				if (std::find(methods.begin(), methods.end(), GET) != methods.end())
					throw std::runtime_error("GET method is already set");
				methods.push_back(GET);
			}
			else if (*it2 == "POST")
			{
				if (std::find(methods.begin(), methods.end(), POST) != methods.end())
					throw std::runtime_error("POST method is already set");
				methods.push_back(POST);
			}
			else if (*it2 == "DELETE")
			{
				if (std::find(methods.begin(), methods.end(), DELETE) != methods.end())
					throw std::runtime_error("DELETE method is already set");
				methods.push_back(DELETE);
			}
		}
	}
}

static void setCgi(std::map<std::string, std::string> &cgiParams, std::vector<std::string> split)
{
	if (split.size() != 2)
		throw std::runtime_error("cgi directive must have an extension and a file");
	if (split[0] != ".py" && split[0] != ".php" && split[0] != ".pl" && split[0] != ".cgi")
		throw std::runtime_error("cgi extension must be .py, .php, .pl or .cgi");
	if (cgiParams.find(split[0]) == cgiParams.end())
		cgiParams[split[0]] = split[1];
	else
		throw std::runtime_error("double cgi directive for the same extension : " + split[0]);
}

/* TODO : change type variable and it's not allowed to have duplicates */
static void setReturn(std::map<int, std::string> &redirects, bool &redirection, std::vector<std::string> split)
{
	std::stringstream ss;
	int code;

	if (!redirects.empty())
		throw std::runtime_error("return directive must be unique");
	redirection = false;
	if (split.size() == 1 && !isHttpCode(split[0]))
	{
		redirects[307] = split[0];
		redirection = true;
	}
	else if (split.size() == 2 && isHttpCode(split[0]))
	{
		ss << split[0];
		ss >> code;
		redirects[code] = split[1];
		redirection = true;
	}
	else
		throw std::runtime_error("return directive must have a code and an uri or only an uri");
}

static void setErrorPage(std::map<std::string, std::string> &errorPages, std::vector<std::string> split)
{
	if (split.size() < 2)
		throw std::runtime_error("error_page directive must have a code and an uri");
	if (isHttpCode(split[split.size() - 1]))
		throw std::runtime_error("error_page uri is missing at the end of the directive");
	for (size_t i = 0; (i < split.size() && isHttpCode(split[i])); i++)
	{
		if (errorPages.find(split[i]) == errorPages.end())
			errorPages[split[i]] = split[split.size() - 1];
	}
}

static bool isServerBlock(std::string const &line)
{
	std::vector<std::string> split = stringSplit(line, ' ');

	if (split[0] == "server" && split[1] == "{")
		return true;
	return false;
}

static bool isLocationBlock(std::string const &line)
{
	std::vector<std::string> split = stringSplit(line, ' ');

	if (line.find("location"))
		return false;
	if (split.size() != 3)
		throw std::runtime_error("Location block usage : location PATH {/*directives*/}");
	if (split[0] == "location" && split[2] == "{")
		return true;
	return false;
}

static bool isDirective(std::string const &line)
{
	std::stringstream ss(line);
	std::string word;
	std::string dir[13] = {
		"listen", "server_name", "index", "root",
		"client_max_body_size", "autoindex", "error_page",
		"return", "cgi", "allowed_methods", "alias",
		"path_info", "upload_location"};

	if (isServerBlock(line))
		throw std::runtime_error("A server block need to be closed before starting new one");
	if (line.at(line.size() - 1) != ';')
		throw std::runtime_error("Directive must end with a semicolon");
	ss >> word;
	for (int i = 0; i < 13; i++)
	{
		if (word == dir[i] || word == dir[i] + ";")
			return true;
	}
	return false;
}

static void trimWhiteSpaces(std::string &str)
{
	str.erase(str.find_last_not_of(" \t") + 1);
	str.erase(0, str.find_first_not_of(" \t"));
}

static bool isLineToIgnore(std::string line)
{
	if (line.empty())
		return true;
	if (line.find_first_not_of(" \t\r") == std::string::npos)
		return true;
	if (line.at(0) == '#')
		return true;
	return false;
}

static void setBooleans(std::string const &key, std::string const &value, bool &toFill, std::vector<std::string> split)
{
	if (split.size() != 1)
		throw std::runtime_error("This directive '" + key + "' allow \"on\" or \"off\"");
	if (value != "on" && value != "off")
		throw std::runtime_error("Invalid value '" + value + "'");
	toFill = false;
	if (value == "on")
		toFill = true;
}

static void inheritanceServerToLocations(std::vector<ServerBlock> &m_serverBlocks)
{
	for (std::vector<ServerBlock>::iterator it = m_serverBlocks.begin(); it != m_serverBlocks.end(); ++it)
	{
		for (std::vector<LocationBlock>::iterator it2 = it->locationBlocks.begin(); it2 != it->locationBlocks.end(); ++it2)
		{
			if (it2->root.empty())
				it2->root = it->root;
			if (it2->clientMaxBodySize.value.empty())
			{
				it2->clientMaxBodySize.value = it->clientMaxBodySize.value;
				it2->clientMaxBodySize.unit = it->clientMaxBodySize.unit;
				it2->bodySize = it->bodySize;
			}
			if (it2->autoindexDone == false)
				it2->autoindex = it->autoindex;
			if (it2->indexes.empty())
				it2->indexes = it->indexes;
			if (it2->errorPages.empty())
				it2->errorPages = it->errorPages;
			if (it2->redirects.empty())
			{
				it2->redirects = it->redirects;
				it2->redirection = it->redirection;
			}
			if (it2->cgiParams.empty())
				it2->cgiParams = it->cgiParams;
			if (it2->methods.empty())
				it2->methods = it->methods;
		}
	}
}

static void setListen(std::string const &value, ServerBlock &serverBlock, std::vector<std::string> split)
{
	std::stringstream ss;
	std::string port;

	if (serverBlock.listenHasBeenSet)
		throw std::runtime_error("Listen directive must be unique inside a server block");
	if (split.size() != 1)
		throw std::runtime_error("Listen directive need only one value");
	if (value.empty() || value.find(' ') != std::string::npos)
		throw std::runtime_error("Invalid value'" + value + "'");
	if (value.at(0) == ':')
		throw std::runtime_error("wrong format for listen directive");
	if (value.find_first_of(':') != std::string::npos)
	{
		if (value.find_first_of(':') != value.find_last_of(':'))
			throw std::runtime_error("Invalid value '" + value + "'");
	}
	if (value.find_first_of(':') != std::string::npos)
	{
		serverBlock.hostPort.first = value.substr(0, value.find(':'));
		if (!isValidHost(serverBlock.hostPort.first))
			throw std::runtime_error("Invalid host '" + serverBlock.hostPort.first + "'");
		ss << value.substr(value.find(':') + 1);
		if (ss.str().find_first_not_of("0123456789") != std::string::npos)
			throw std::runtime_error("Port must be a number.");
		ss >> serverBlock.hostPort.second;
	}
	else
	{
		ss << value;
		if (ss.str().find_first_not_of("0123456789") != std::string::npos)
			throw std::runtime_error("Port must be a number.");
		ss >> serverBlock.hostPort.second;
	}
	serverBlock.listenHasBeenSet = true;
}

static void setStringValue(std::string const &key, std::string &valueToSet, std::vector<std::string> split)
{
	if (split.size() != 1)
		throw std::runtime_error("This directive '" + key + "' need only one value");
	valueToSet = split[0];
}

static void pushSplit(std::vector<std::string> &dst, std::vector<std::string> toPush)
{
	if (dst.empty())
	{
		dst = toPush;
		return;
	}
	for (std::vector<std::string>::iterator it = toPush.begin(); it != toPush.end(); ++it)
		dst.push_back(*it);
}

void Configuration::parseLocationDirective(std::string &line, LocationBlock &locationBlock)
{
	std::vector<std::string> split;
	std::string str = line;
	std::string key;

	if (str.at(str.size() - 1) != ';')
		throw std::runtime_error("Directive '" + str + "' must end with a semicolon");
	str.erase(str.size() - 1);
	split = stringSplit(str, ' ');
	if (split.size() == 0)
		throw std::runtime_error("Invalid directive '" + str + "'");
	key = split[0];
	split.erase(split.begin());
	if (key == "alias")
		setStringValue(key, locationBlock.alias, split);
	else if (key == "root")
		setStringValue(key, locationBlock.root, split);
	else if (key == "client_max_body_size")
		setClientMaxBodySize(locationBlock.clientMaxBodySize, locationBlock.bodySize, split);
	else if (key == "autoindex")
	{
		setBooleans(key, split[0], locationBlock.autoindex, split);
		locationBlock.autoindexDone = true;
	}
	else if (key == "path_info")
		setBooleans(key, split[0], locationBlock.pathInfo, split);
	else if (key == "index")
		pushSplit(locationBlock.indexes, split);
	else if (key == "error_page")
		setErrorPage(locationBlock.errorPages, split);
	else if (key == "upload_location")
		setStringValue(key, locationBlock.uploadLocation, split);
	else if (key == "return")
		setReturn(locationBlock.redirects, locationBlock.redirection, split);
	else if (key == "cgi")
		pushSplit(locationBlock.cgiExtensions, split);
	else if (key == "cgi_param")
		setCgi(locationBlock.cgiParams, split);
	else if (key == "allowed_methods")
		setAllowedMethods(locationBlock.methods, split);
	else
		throw std::runtime_error("Unknown directive '" + key + "'");
}

void Configuration::parseLocationBlock(ServerBlock &serverBlock, std::string const &locationLine, std::stringstream &ss)
{
	std::vector<std::string> split = stringSplit(locationLine, ' ');
	LocationBlock locationBlock;
	std::string line;

	initLocationBlock(locationBlock);
	locationBlock.path = split[1];
	while (std::getline(ss, line))
	{
		trimWhiteSpaces(line);
		if (line.empty() || line.at(0) == '#')
			continue;
		else if (line == "}")
			break;
		else
			parseLocationDirective(line, locationBlock);
	}
	serverBlock.locationBlocks.push_back(locationBlock);
}

void Configuration::parseServerDirective(std::string const &line, ServerBlock &serverBlock)
{
	std::vector<std::string> split;
	std::string str = line;
	std::string key;

	if (str.at(str.size() - 1) != ';')
		throw std::runtime_error("Directive '" + str + "' must end with a semicolon");
	str.erase(str.size() - 1);
	split = stringSplit(str, ' ');
	if (split.size() == 0)
		throw std::runtime_error("Invalid directive '" + str + "'");
	key = split[0];
	split.erase(split.begin());
	if (key == "autoindex")
		setBooleans(key, split[0], serverBlock.autoindex, split);
	else if (key == "root")
		setStringValue(key, serverBlock.root, split);
	else if (key == "client_max_body_size")
		setClientMaxBodySize(serverBlock.clientMaxBodySize, serverBlock.bodySize, split);
	else if (key == "listen")
		setListen(split[0], serverBlock, split);
	else if (key == "server_name")
		serverBlock.serverNames = split;
	else if (key == "index")
		pushSplit(serverBlock.indexes, split);
	else if (key == "error_page")
		setErrorPage(serverBlock.errorPages, split);
	else if (key == "return")
		setReturn(serverBlock.redirects, serverBlock.redirection, split);
	else if (key == "cgi")
		pushSplit(serverBlock.cgiExtensions, split);
	else if (key == "cgi_param")
		setCgi(serverBlock.cgiParams, split);
	else if (key == "allowed_methods")
		setAllowedMethods(serverBlock.methods, split);
	else
		throw std::runtime_error("Unknown directive '" + key + "'");
}

void Configuration::parseServerBlock(std::stringstream &ss)
{
	ServerBlock server;
	std::string line;

	curlyBrackets++;
	initServerBlock(server);
	while (std::getline(ss, line))
	{
		trimWhiteSpaces(line);
		if (line.empty() || line.at(0) == '#')
			continue;
		else if (isServerBlock(line))
			throw std::runtime_error("No server blocks are allowed inside another server block");
		else if (line == "}")
		{
			curlyBrackets--;
			break;
		}
		else if (isLocationBlock(line))
			parseLocationBlock(server, line, ss);
		else if (isDirective(line))
			parseServerDirective(line, server);
		else
			throw std::runtime_error("Unknown directive at this line : [" + line + "]");
	}
	if (curlyBrackets != 0)
		throw std::runtime_error("A server block need to be closed before opening a new one");
	pushServerBlock(m_serverBlocks, server);
}

Configuration::Configuration(std::string const &t_configFile) : m_configFile(t_configFile), curlyBrackets(0)
{
	std::ifstream file(m_configFile.c_str());
	std::string line;
	std::stringstream ss;

	if (!file)
		throw std::runtime_error("Cannot open file " + m_configFile);
	ss << file.rdbuf();
	file.close();
	while (std::getline(ss, line))
	{
		trimWhiteSpaces(line);
		if (line.empty() || line.at(0) == '#')
			continue;
		else if (!isServerBlock(line))
			throw std::runtime_error("You need to start with a server block instead of '" + line + "'");
		if (curlyBrackets != 0)
			throw std::runtime_error("A server block need to be closed before opening a new one");
		parseServerBlock(ss);
	}
	if (m_serverBlocks.empty())
		throw std::runtime_error("No server block found");
	inheritanceServerToLocations(m_serverBlocks);
}

Configuration::Configuration()
{
	ServerBlock server;

	initServerBlock(server);
	pushServerBlock(m_serverBlocks, server);
}

Configuration::~Configuration()
{
}

std::vector<ServerBlock> const &Configuration::getServerBlocks() const
{
	return m_serverBlocks;
}

int Configuration::getBodySize(BodySize const &bodySize)
{
	std::stringstream ss;
	int weight;

	ss << bodySize.value;
	ss >> weight;
	if (bodySize.unit == "K")
		weight *= 1024;
	else if (bodySize.unit == "M")
		weight *= 1024 * 1024;
	else if (bodySize.unit == "G")
		weight *= 1024 * 1024 * 1024;
	return weight;
}

std::vector<int> Configuration::getPorts() const
{
	std::vector<int> ports;

	for (std::vector<ServerBlock>::const_iterator it = m_serverBlocks.begin(); it != m_serverBlocks.end(); ++it)
		ports.push_back(it->hostPort.second);
	return ports;
}

void Configuration::printConfig() const
{
	for (std::vector<ServerBlock>::const_iterator it = m_serverBlocks.begin(); it != m_serverBlocks.end(); ++it)
	{
		std::cout << "\033[0;33;42m----- SERVER -----\033[0m" << std::endl;

		// LISTEN
		std::cout << "port: " << it->hostPort.second << std::endl;
		std::cout << "host: " << it->hostPort.first << std::endl;

		// SERVER NAMES
		std::cout << "serverNames: ";
		for (std::vector<std::string>::const_iterator it2 = it->serverNames.begin(); it2 != it->serverNames.end(); ++it2)
			std::cout << *it2 << " ";
		std::cout << std::endl;

		// ROOT
		std::cout << "root: " << it->root << std::endl;

		// ERROR PAGES
		std::cout << "errorPages: ";
		for (std::map<std::string, std::string>::const_iterator it2 = it->errorPages.begin(); it2 != it->errorPages.end(); ++it2)
			std::cout << it2->first << ":" << it2->second << " ";
		std::cout << std::endl;

		// CLIENT MAX BODY SIZE
		std::cout << "clientMaxBodySize: " << it->clientMaxBodySize.value << " " << it->clientMaxBodySize.unit << std::endl;
		std::cout << "bodySize: " << it->bodySize << std::endl;

		// AUTOINDEX
		std::cout << "autoindex: " << (it->autoindex ? "on" : "off") << std::endl;

		// INDEXES
		std::cout << "indexes: ";
		for (std::vector<std::string>::const_iterator it2 = it->indexes.begin(); it2 != it->indexes.end(); ++it2)
			std::cout << *it2 << " ";
		std::cout << std::endl;

		// ERROR PAGES
		std::cout << "errorPages: ";
		for (std::map<std::string, std::string>::const_iterator it2 = it->errorPages.begin(); it2 != it->errorPages.end(); ++it2)
			std::cout << it2->first << ":" << it2->second << " ";
		std::cout << std::endl;

		// REDIRECTS
		std::cout << "redirection : " << (it->redirection ? "on" : "off") << std::endl;
		std::cout << "redirects: ";
		for (std::map<int, std::string>::const_iterator it2 = it->redirects.begin(); it2 != it->redirects.end(); ++it2)
			std::cout << it2->first << " " << it2->second << " ";
		std::cout << std::endl;

		// CGI PARAMS
		std::cout << "cgiParams: ";
		for (std::map<std::string, std::string>::const_iterator it2 = it->cgiParams.begin(); it2 != it->cgiParams.end(); ++it2)
			std::cout << it2->first << " " << it2->second << " ";
		std::cout << std::endl;

		// METHODS
		std::cout << "methods: ";
		for (std::vector<http_method>::const_iterator it2 = it->methods.begin(); it2 != it->methods.end(); ++it2)
			std::cout << *it2 << " ";
		std::cout << std::endl;

		// LOCATION BLOCKS
		for (std::vector<LocationBlock>::const_iterator it2 = it->locationBlocks.begin(); it2 != it->locationBlocks.end(); ++it2)
		{
			std::cout << "--LOCATION--" << std::endl;
			std::cout << "  path: " << it2->path << std::endl;
			std::cout << "  alias: " << it2->alias << std::endl;
			std::cout << "  root: " << it2->root << std::endl;
			std::cout << "  clientMaxBodySize: " << it2->clientMaxBodySize.value << " " << it2->clientMaxBodySize.unit << std::endl;
			std::cout << "  body size: " << it2->bodySize << std::endl;
			std::cout << "  autoindex: " << (it2->autoindex ? "on" : "off") << std::endl;
			std::cout << "  pathInfo: " << (it2->pathInfo ? "on" : "off") << std::endl;

			std::cout << "  indexes: ";
			for (std::vector<std::string>::const_iterator it3 = it2->indexes.begin(); it3 != it2->indexes.end(); ++it3)
				std::cout << *it3 << " ";
			std::cout << std::endl;

			std::cout << "  errorPages: ";
			for (std::map<std::string, std::string>::const_iterator it3 = it2->errorPages.begin(); it3 != it2->errorPages.end(); ++it3)
				std::cout << it3->first << ":" << it3->second << " ";
			std::cout << std::endl;

			std::cout << "  uploadLocation: " << it2->uploadLocation << std::endl;

			std::cout << "  redirects: ";
			for (std::map<int, std::string>::const_iterator it3 = it2->redirects.begin(); it3 != it2->redirects.end(); ++it3)
				std::cout << it3->first << " " << it3->second << " ";
			std::cout << std::endl;

			std::cout << "  redirection: " << (it2->redirection ? "on" : "off") << std::endl;

			std::cout << "  cgiParams: ";
			for (std::map<std::string, std::string>::const_iterator it3 = it2->cgiParams.begin(); it3 != it2->cgiParams.end(); ++it3)
				std::cout << it3->first << " " << it3->second << " ";
			std::cout << std::endl;

			std::cout << "  cgi Extensions: ";
			for (std::vector<std::string>::const_iterator it3 = it2->cgiExtensions.begin(); it3 != it2->cgiExtensions.end(); ++it3)
				std::cout << *it3 << " ";
			std::cout << std::endl;

			std::cout << "  methods (" << it2->methods.size() << "): ";
			for (std::vector<http_method>::const_iterator it3 = it2->methods.begin(); it3 != it2->methods.end(); ++it3)
				std::cout << *it3 << " ";
			std::cout << std::endl;
		}
		std::cout << "\033[0;33;42m------------------\033[0m" << std::endl;
	}
}