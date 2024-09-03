#include "../include/Configuration.hpp"
#include "Configuration.hpp"
#include <string>

static int	isEmptyLine(std::string line)
{
	return (line.empty() || line.erase(line.find_last_not_of(" \t\r" ) + 1).empty() || line[0] == '#');
}

void	Configuration::initDirectiveMap()
{
	m_directives["listen"] = LISTEN;
	m_directives["server_name"] = SERVER_NAME;
	m_directives["root"] = ROOT;
	m_directives["error_page"] = ERROR_PAGE;
	m_directives["client_max_body_size"] = CLIENT_MAX_BODY_SIZE;
	m_directives["location"] = LOCATION;
	m_directives["autoindex"] = AUTOINDEX;
	m_directives["index"] = INDEX;
	m_directives["return"] = RETURN;
	m_directives["path_info"] = PATH_INFO;
	m_directives["cgi"] = CGI;
	m_directives["alias"] = ALIAS;
	m_directives["upload_location"] = UPLOAD_LOCATION;
	m_directives["set_method"] = SET_METHOD;
}

static void	initLocationBlock(LocationBlock &locationBlock)
{
	locationBlock.exactMatch = false;
	locationBlock.path = "";
	locationBlock.root = "";
	locationBlock.alias = "";
	locationBlock.clientMaxBodySize.value = "";
	locationBlock.clientMaxBodySize.unit = "";
	locationBlock.autoindex = false;
	locationBlock.indexes.clear();
	locationBlock.redirects.clear();
	locationBlock.pathInfo = false;
	locationBlock.cgiParams.clear();
	locationBlock.uploadLocation = "";
	locationBlock.methods.clear();
}

static void	initServerBlock(ServerBlock &serverBlock)
{
	serverBlock.port = 8080;
	serverBlock.host = "localhost";
	serverBlock.serverNames.clear();
	serverBlock.root = "/";
	serverBlock.errorPages.clear();
	serverBlock.clientMaxBodySize.value = "1";
	serverBlock.clientMaxBodySize.unit = "M";
	serverBlock.locationBlocks.clear();
}

static void	setLocationDefaultValues(ServerBlock &serverBlock, LocationBlock &locationBlock)
{
	if (locationBlock.root.empty())
		locationBlock.root = serverBlock.root;
	if (locationBlock.clientMaxBodySize.value.empty())
	{
		locationBlock.clientMaxBodySize.value = serverBlock.clientMaxBodySize.value;
		locationBlock.clientMaxBodySize.unit = serverBlock.clientMaxBodySize.unit;
	}
}

std::string	Configuration::extractValue(std::string const &line)
{
	std::istringstream	iss(line);
	std::string			value;

	std::getline(iss, value);
	if (value.empty())
		throw std::runtime_error("Directive '" + line + "' must have a value");
	if (value[value.size() - 1] != ';')
		throw std::runtime_error("Directive '" + line + "' must end with a semicolon");
	value.erase(0, value.find_first_not_of(" \t"));
	value[value.size() - 1] = '\0';
	return (value);
}

std::string Configuration::extractDirective(std::string const &line)
{
	std::istringstream	iss(line);
	std::string			directive;

	iss >> directive;
	directive.erase(0, directive.find_first_not_of(" \t"));
	return (directive);
}

void Configuration::setListen(std::string const &value, ServerBlock &serverBlock)
{
	if (value.empty() || value.find(' ') != std::string::npos)
		throw std::runtime_error("[setListen]Invalid value'" + value + "'");
	if (value.find(':') != std::string::npos)
	{
		serverBlock.host = value.substr(0, value.find(':'));
		serverBlock.port = std::atoi(value.substr(value.find(':') + 1).c_str());
	}
	else
		serverBlock.port = std::atoi(value.c_str());
}

/*
The server_name directive tells the server:
"If someone requests example.com, this server should handle it."
*/
void Configuration::setName(std::string const &value, ServerBlock &serverBlock)
{
	std::istringstream	iss(value);
	std::string			word;


	while (iss >> word)
		serverBlock.serverNames.push_back(word);
}

void Configuration::setErrorPage(std::string const &value, ServerBlock &serverBlock)
{
}

void	Configuration::setAlias(std::string const &value, LocationBlock &locationBlock)
{
	// std::cout << "INSIDE SET ALIAS : [" << value << "]" << std::endl;
	if (value.empty() || value.find(' ') != std::string::npos)
		throw std::runtime_error("[setAlias]Invalid value'" + value + "'");
	locationBlock.alias = value;
}

void	Configuration::setAutoindex(std::string const &value, LocationBlock &locationBlock)
{
}

void	Configuration::setIndex(std::string const &value, LocationBlock &locationBlock)
{
}

void	Configuration::setRedirect(std::string const &value, LocationBlock &locationBlock)
{
}

void	Configuration::setPathInfo(std::string const &value, LocationBlock &locationBlock)
{
}

void	Configuration::setCgi(std::string const &value, LocationBlock &locationBlock)
{
}

void	Configuration::setUploadLocation(std::string const &value, LocationBlock &locationBlock)
{
	// std::cout << "INSIDE SET UPLOADLOCATION : [" << value << "]" << std::endl;
	// if I found a space inside the value, I will throw an exception
	if (value.empty() || value.find(' ') != std::string::npos)
		throw std::runtime_error("[setUploadLocation]Invalid value'" + value + "'");
	locationBlock.uploadLocation = value;
}

void	Configuration::setMethod(std::string const &value, LocationBlock &locationBlock)
{
}

void	Configuration::setServerClientMaxBodySize(std::string const &value, ServerBlock &serverBlock)
{
}

void	Configuration::setLocationRoot(std::string const &value, LocationBlock &locationBlock)
{
	// std::cout << "INSIDE SET LOCATIONROOT : [" << value << "]" << std::endl;
	if (value.empty() || value.find(' ') != std::string::npos)
		throw std::runtime_error("[setLocationRoot]Invalid value'"+ value + "'");
	if (!value.empty())
		locationBlock.root = value;
}
void	Configuration::setLocationClientMaxBodySize(std::string const &value, LocationBlock &locationBlock)
{
}

void	Configuration::setServerValues(std::string const &key, std::string const &value, ServerBlock &serverBlock)
{
	std::cout << "{" << key << "} : [" << value << "]" << std::endl;

	if (key == "listen")
		setListen(value, serverBlock);
	else if (key == "server_name")
		setName(value, serverBlock);
	else if (key == "root")
	{
		if (value.find_first_of(" \t\n\v\f\r") == std::string::npos)
			throw std::runtime_error("root directive must have only one value or none");
		serverBlock.root = value;
	}
	else if (key == "error_page")
		setErrorPage(value, serverBlock);
	else if (key == "client_max_body_size")
		setServerClientMaxBodySize(value, serverBlock);
	else
		throw std::runtime_error("Unknown directive '" + key + "'");
}

void	Configuration::setLocationValues(std::string const &expression, std::string const &value, LocationBlock &locationBlock)
{
	directives d = m_directives[expression];

	// if I found a space inside the value of a directive that should not have one
	if ((d == ROOT || d == ALIAS || d == UPLOAD_LOCATION) && value.find(' ') != std::string::npos)
		throw std::runtime_error("[setLocationValues]Invalid value'" + value + "'");
	switch (d) {
		case ROOT:
			// locationBlock.root = value;
			setLocationRoot(value, locationBlock);
			break;

		case ALIAS:
			// locationBlock.alias = value;
			setAlias(value, locationBlock);
			break;

		case UPLOAD_LOCATION:
			locationBlock.uploadLocation = value;
			break;

		case CLIENT_MAX_BODY_SIZE:
			// std::cout << "\tclient_max_body_size: [" << value << "]" << std::endl;
			// setLocationClientMaxBodySize(value, locationBlock);
			break;

		case AUTOINDEX:
			// std::cout << "\tautoindex: [" << value << "]" << std::endl;
			// setAutoindex(value, locationBlock);
			break;

		case INDEX:
			// std::cout << "\tindex: [" << value << "]" << std::endl;
			// setIndex(value, locationBlock);
			break;

		case RETURN:
			// std::cout << "\treturn: [" << value << "]" << std::endl;
			// setRedirect(value, locationBlock);
			break;

		case PATH_INFO:
			// std::cout << "\tpath_info: [" << value << "]" << std::endl;
			// setPathInfo(value, locationBlock);
			break;

		case CGI:
			// std::cout << "\tcgi_param: [" << value << "]" << std::endl;
			// setCgi(value, locationBlock);
			break;

		case SET_METHOD:
			// std::cout << "\tmethod: [" << value << "]" << std::endl;
			// setMethod(value, locationBlock);
			break;

		default:
			throw std::runtime_error("[setLocationValues]Unknown directive '" + expression + "'");
			break;
	}
}

void	Configuration::parseLocationBlock(std::stringstream &content, ServerBlock &serverBlock, std::string const &line)
{
	LocationBlock	locationBlock;
	std::string		row;
	std::string		directive;
	std::string		value;

	initLocationBlock(locationBlock);
	locationBlock.path = line;
	if (locationBlock.path[locationBlock.path.size() - 1] != '{' || locationBlock.path[locationBlock.path.size() - 2] != ' ')
		throw std::runtime_error("[parseLocationBlock]Invalid location block '" + locationBlock.path + "'");
	locationBlock.path.erase(locationBlock.path.size() - 1);
	if (locationBlock.path.empty() || isEmptyLine(locationBlock.path))
		throw std::runtime_error("[parseLocationBlock]Invalid location block '" + locationBlock.path + "'");
	locationBlock.path[locationBlock.path.size() - 1] = '\0';
	if (locationBlock.path.find(' ') != std::string::npos)
		throw std::runtime_error("[parseLocationBlock]Invalid location block '" + locationBlock.path + "'");
	while (std::getline(content, row))
	{
		if (isEmptyLine(row))
			continue;
		if (row == "\t}")
			break;
		else
		{
			directive = extractDirective(row);
			value = extractValue(row);
			value.erase(0, directive.size() + 1);
			setLocationValues(extractDirective(row), value, locationBlock);
		}
	}
	setLocationDefaultValues(serverBlock, locationBlock);
	serverBlock.locationBlocks.push_back(locationBlock);
}

void	Configuration::parseServerDirective(std::string const &line, ServerBlock &serverBlock)
{
	std::string	dir = line.substr(line.find_first_not_of(" \t"), line.size());
	std::string	keys[5] = {"listen", "server_name", "root", "error_page", "client_max_body_size"};
	std::string	value;

	for (int i = 0; i < 5; i++)
	{
		if (!dir.rfind(keys[i], 0))
		{
			value = dir.substr(keys[i].size());
			value = value.substr(value.find_first_not_of(" \t"));
			if (value[value.size() - 1] != ';')
				throw std::runtime_error("[parseServerDirective]Directive '" + dir + "' must end with a semicolon");
			value[value.size() - 1] = '\0';
			// std::cout << "{" << keys[i] << "} : [" << value << "]" << std::endl;
			setServerValues(keys[i], value, serverBlock);


			return;
		}
	}
	throw std::runtime_error("[parseServerDirective]Unknown directive '" + dir + "'");
}

void	Configuration::parseServerBlock(std::stringstream &content)
{
	ServerBlock	server;
	std::string	line;
	std::string	directive;
	std::string	value;

	initServerBlock(server);
	while (std::getline(content, line))
	{
		if (isEmptyLine(line))
			continue;
		else if (line == "}")
			break;
		else if (line.rfind("\tlocation", 0) == 0)
			parseLocationBlock(content, server, line.substr(10));
		else
		{
			parseServerDirective(line, server);
			// directive = extractDirective(line);
			// value = extractValue(line);
			// value.erase(0, directive.size() + 1);
			// setServerValues(directive, value, server);
		}
	}
	// std::cout << "server name: " << server.serverNames.size() << std::endl;
	if (server.serverNames.empty())
	{
		// std::cout << "no server name found, adding default one" << std::endl;
		server.serverNames.push_back("webserv");
	}
	// else
		// std::cout << "server name found: " << server.serverNames[0] << std::endl;
	m_serverBlocks.push_back(server);
}

Configuration::Configuration(std::string const &t_configFile) : m_configFile(t_configFile)
{
	std::ifstream	file(m_configFile.c_str());
	std::string		line;

	initDirectiveMap();
	if (!file)
		throw std::runtime_error("Cannot open file " + m_configFile);
	this->m_content << file.rdbuf();
	file.close();
	while (std::getline(this->m_content, line))
	{
		if (isEmptyLine(line))
			continue;
		else if (line == "server {")
			parseServerBlock(this->m_content);
		else
			throw std::runtime_error("[parseConfigFile]Unknown directive '" + line + "'");
	}
}

Configuration::Configuration()
{
}

Configuration::~Configuration()
{
}


void	Configuration::printConfig() const
{
	for(std::vector<ServerBlock>::const_iterator it = m_serverBlocks.begin(); it != m_serverBlocks.end(); ++it)
	{
		std::cout << "\033[0;33;42m----- SERVER -----\033[0m" << std::endl;
		std::cout << "port: " << it->port << std::endl;
		std::cout << "host: " << it->host << std::endl;
		std::cout << "serverNames: ";
		for (std::vector<std::string>::const_iterator it2 = it->serverNames.begin(); it2 != it->serverNames.end(); ++it2)
			std::cout << *it2 << " ";
		std::cout << std::endl;
		std::cout << "root: " << it->root << std::endl;
		std::cout << "errorPages: ";
		for (std::map<std::string, std::string>::const_iterator it2 = it->errorPages.begin(); it2 != it->errorPages.end(); ++it2)
			std::cout << it2->first << " " << it2->second << " ";
		std::cout << std::endl;
		std::cout << "clientMaxBodySize: " << it->clientMaxBodySize.value << " " << it->clientMaxBodySize.unit << std::endl;
		for (std::vector<LocationBlock>::const_iterator it2 = it->locationBlocks.begin(); it2 != it->locationBlocks.end(); ++it2)
		{
			std::cout << "--LOCATION--" << std::endl;
			std::cout << "  exactMatch: " << it2->exactMatch << std::endl;
			std::cout << "  path: " << it2->path << std::endl;
			std::cout << "  root: " << it2->root << std::endl;
			std::cout << "  alias: " << it2->alias << std::endl;
			std::cout << "  clientMaxBodySize: " << it2->clientMaxBodySize.value << " " << it2->clientMaxBodySize.unit << std::endl;
			std::cout << "  autoindex: " << it2->autoindex << std::endl;
			std::cout << "  indexes: ";
			for (std::vector<std::string>::const_iterator it3 = it2->indexes.begin(); it3 != it2->indexes.end(); ++it3)
				std::cout << *it3 << " ";
			std::cout << std::endl;
			std::cout << "  redirects: ";
			for (std::map<std::string, std::string>::const_iterator it3 = it2->redirects.begin(); it3 != it2->redirects.end(); ++it3)
				std::cout << it3->first << " " << it3->second << " ";
			std::cout << std::endl;
			std::cout << "  pathInfo: " << it2->pathInfo << std::endl;
			std::cout << "  cgiParams: ";
			for (std::map<std::string, std::string>::const_iterator it3 = it2->cgiParams.begin(); it3 != it2->cgiParams.end(); ++it3)
				std::cout << it3->first << " " << it3->second << " ";
			std::cout << std::endl;
			std::cout << "  uploadLocation: " << it2->uploadLocation << std::endl;
			std::cout << "  methods: ";
			for (std::vector<http_method>::const_iterator it3 = it2->methods.begin(); it3 != it2-> methods.end(); ++it3)
				std::cout << *it3 << " ";
			std::cout << std::endl;
		}
		std::cout << "\033[0;33;42m------------------\033[0m" << std::endl;
	}
}
