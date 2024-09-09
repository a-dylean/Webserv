#include "../include/Configuration.hpp"
#include "Configuration.hpp"

static std::string	getLocationPath(std::string const &line)
{
	// std::string	res = line;

	// if (res.compare(" {") == 0)
	// 	throw std::runtime_error("Invalid location path");
	// res = res.substr(res.find_first_not_of(" \t"));
	// res[res.size() - 1] = '\0';
	// res[res.size() - 2] = '\0';
	// if (res.empty() || res[0] == '\0')
	// 	throw std::runtime_error("Invalid location path");
	// if (res.find(' ') != std::string::npos)
	// 	throw std::runtime_error("Invalid location path");
	// return res;
	//Chat generated this code:
	std::string res = line;
    res.erase(0, res.find_first_not_of(" \t"));
    res.erase(res.find_last_not_of(" \t") - 1);
    if (res.empty() || res == "{" || res.find(' ') != std::string::npos)
    {
        throw std::runtime_error("Invalid location path");
    }
    return res;
}

static bool	isEmptyLine(std::string line)
{
	if (line.empty())
		return true;
	if (line.find_first_not_of(" \t\r") == std::string::npos)
		return true;
	if (line[0] == '#')
		return true;
	return false;
}

static bool	isOnOrOff(std::string const &value)
{
	if (value != "on" && value != "off")
		throw std::runtime_error("Invalid value '" + value + "'");
	return (value == "on");
}

static BodySize	createBodySize(std::string const &value)
{
	BodySize	bodySize;
	std::string	unit;
	std::string	number;

	if (value.empty() || value[0] == '\0')
		throw std::runtime_error("client_max_body_size no value");
	number = value.substr(0, value.find_first_not_of("0123456789"));
	if (number.empty() || number[0] == '\0')
		throw std::runtime_error("client_max_body_size no number found");
	unit = value.substr(value.find_first_not_of("0123456789"));
	if (unit.size() != 1)
		throw std::runtime_error("client_max_body_size invalid unit size");
	if (unit[0] == 'K' || unit[0] == 'k')
		unit = "K";
	else if (unit[0] == 'M' || unit[0] == 'm')
		unit = "M";
	else if (unit[0] == 'G' || unit[0] == 'g')
		unit = "G";
	else
		throw std::runtime_error("client_max_body_size invalid unit (k/K, m/M, g/G)");
	bodySize.value = number;
	bodySize.unit = unit;
	return bodySize;
}

static void	initLocationBlock(LocationBlock &locationBlock)
{
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
	serverBlock.root = "./www";
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

void Configuration::setListen(std::string const &value, ServerBlock &serverBlock)
{
	// TODO : listen localhost; but I have port at 0.
	if (value.empty() || value.find(' ') != std::string::npos)
		throw std::runtime_error("[setListen]Invalid value'" + value + "'");
	if (value.find_first_of(':') != std::string::npos)
	{
		serverBlock.host = value.substr(0, value.find(':'));
		serverBlock.port = std::atoi(value.substr(value.find(':') + 1).c_str());
	}
	else
		serverBlock.port = std::atoi(value.c_str());
}

void Configuration::addErrorPage(std::string const &value, ServerBlock &serverBlock)
{
	std::string	code;
	std::string	uri;

	if (value.empty() || value.find_first_of(" \t") == std::string::npos)
		throw std::runtime_error("error_page wrong format.");
	code = value.substr(0, value.find_first_of(" \t"));
	if (code.find_first_not_of("0123456789") != std::string::npos)
		throw std::runtime_error("error_page code must be a number.");
	// TODO : check if code is a valid http code
	uri = value.substr(value.find_first_not_of(" \t", code.size()));
	if (uri.empty() || uri[0] == '\0')
		throw std::runtime_error("error_page uri is empty.");
	if (uri.find_first_of(" \t") != std::string::npos)
		throw std::runtime_error("error_page uri must be a single value.");
	serverBlock.errorPages[code] = uri;
}

void	Configuration::setRedirect(std::string const &value, LocationBlock &locationBlock)
{
}

void	Configuration::setCgi(std::string const &value, LocationBlock &locationBlock)
{
}


void	Configuration::setMethod(std::string const &value, LocationBlock &locationBlock)
{
}

static void	parseNames(std::string const &value, std::vector<std::string> &names)
{
	std::istringstream	iss(value);
	std::string			word;

	while (iss >> word && !word.empty())
		names.push_back(word);
}

void	Configuration::setServerValues(std::string const &key, std::string const &value, ServerBlock &serverBlock)
{
	if ((key == "root" || key == "listen" || key == "client_max_body_size") && value.find_first_of(" \t\n\v\f\r") != std::string::npos)
		throw std::runtime_error("Directive '" + key + "' must have only one value");
	if (key == "listen")
	{
		
		setListen(value, serverBlock);
	}
	else if (key == "server_name")
		parseNames(value, serverBlock.serverNames);
	else if (key == "root")
		serverBlock.root = value;
	else if (key == "error_page")
		addErrorPage(value, serverBlock);
	else if (key == "client_max_body_size")
		serverBlock.clientMaxBodySize = createBodySize(value);
	else
		throw std::runtime_error("[setServerValues]Unknown directive '" + key + "'");
}

void	Configuration::setLocationValues(std::string const &key, std::string const &value, LocationBlock &locationBlock)
{
	if ((key == "root" || key == "alias" || key == "client_max_body_size" || key == "autoindex" || key == "path_info" || key == "upload_location") && value.find_first_of(" \t\n\v\f\r") != std::string::npos)
		throw std::runtime_error("Directive '" + key + "' must have only one value");
	if (key == "root")
		locationBlock.root = value;
	else if (key == "alias")
		locationBlock.alias = value;
	else if (key == "client_max_body_size")
		locationBlock.clientMaxBodySize = createBodySize(value);
	else if (key == "autoindex")
		locationBlock.autoindex = isOnOrOff(value);
	else if (key == "index")
		parseNames(value, locationBlock.indexes);
	else if (key == "return")
		setRedirect(value, locationBlock);
	else if (key == "path_info")
		locationBlock.pathInfo = isOnOrOff(value);
	else if (key == "cgi")
		setCgi(value, locationBlock);
	else if (key == "upload_location")
		locationBlock.uploadLocation = value;
	else if (key == "set_method")
		setMethod(value, locationBlock);
	else
		throw std::runtime_error("[setLocationValues]Unknown directive '" + key + "'");
}

void	Configuration::parseLocationDirective(std::string &line, LocationBlock &locationBlock)
{
	std::string	dir = line.substr(line.find_first_not_of(" \t"), line.size());
	std::string	keys[11] = {
		"root", "alias", "client_max_body_size", 
		"autoindex", "index", "return", "path_info", 
		"cgi", "upload_location", "set_method"
	};
	std::string	value;

	for (int i = 0; i < 11; i++)
	{
		if (!dir.rfind(keys[i], 0))
		{
			value = dir.substr(keys[i].size());
			value = value.substr(value.find_first_not_of(" \t"));
			if (value[value.size() - 1] != ';')
				throw std::runtime_error("[parseLocationDirective]Directive '" + dir + "' must end with a semicolon");
			value = value.substr(0, value.size() - 1);
			setLocationValues(keys[i], value, locationBlock);
			return;
		}
	}
}

void	Configuration::parseLocationBlock(std::stringstream &content, ServerBlock &serverBlock, std::string const &line)
{
	LocationBlock	locationBlock;
	std::string		row;
	std::string		directive;
	std::string		value;

	if (line[line.size() - 1] != '{' || line[line.size() - 2] != ' ')
		throw std::runtime_error("Location block must end with a ' {'");
	initLocationBlock(locationBlock);
	locationBlock.path = getLocationPath(line);
	while (std::getline(content, row))
	{
		if (isEmptyLine(row))
			continue;
		if (row == "\t}")
			break;
		else
			parseLocationDirective(row, locationBlock);
	}
	setLocationDefaultValues(serverBlock, locationBlock);
	if (locationBlock.indexes.empty())
		locationBlock.indexes.push_back("default.html");
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
			value = value.substr(0, value.size() - 1);
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
			parseLocationBlock(content, server, line.substr(9));
		else
			parseServerDirective(line, server);
	}
	if (server.serverNames.empty())
		server.serverNames.push_back("webserv");
	m_serverBlocks.push_back(server);
}

Configuration::Configuration()
{
	ServerBlock	server;

	initServerBlock(server);
	m_serverBlocks.push_back(server);
}

Configuration::Configuration(std::string const &t_configFile) : m_configFile(t_configFile)
{
	std::ifstream	file(m_configFile.c_str());
	std::string		line;

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
			throw std::runtime_error("Unknown directive '" + line + "'");
	}
}

Configuration::~Configuration()
{
}

std::vector<ServerBlock> const &Configuration::getServerBlocks() const
{
	return m_serverBlocks;
}


// This function can be used to get the body size in bytes
const int	Configuration::getBodySize(BodySize const &bodySize) const
{
	int	weight = std::atoi(bodySize.value.c_str());

	if (bodySize.unit == "K")
		weight *= 1024;
	else if (bodySize.unit == "M")
		weight *= 1024 * 1024;
	else if (bodySize.unit == "G")
		weight *= 1024 * 1024 * 1024;
	return weight;
}

std::vector<int>	Configuration::getPorts() const
{
	std::vector<int>	ports;

	for (std::vector<ServerBlock>::const_iterator it = m_serverBlocks.begin(); it != m_serverBlocks.end(); ++it)
		ports.push_back(it->port);
	return ports;
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
			std::cout << it2->first << ":" << it2->second << " ";
		std::cout << std::endl;
		std::cout << "clientMaxBodySize: " << it->clientMaxBodySize.value << " " << it->clientMaxBodySize.unit << std::endl;
		for (std::vector<LocationBlock>::const_iterator it2 = it->locationBlocks.begin(); it2 != it->locationBlocks.end(); ++it2)
		{
			std::cout << "--LOCATION--" << std::endl;
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

int	getClientMaxBodySize(BodySize const &bodySize)
{
    int maxClientBodySize;
    std::stringstream ss(bodySize.value);
   
    ss >> maxClientBodySize;

	if (bodySize.unit == "K")
		maxClientBodySize *= 1024;
	else if (bodySize.unit == "M")
		maxClientBodySize *= 1024 * 1024;
	else if (bodySize.unit == "G")
		maxClientBodySize *= 1024 * 1024 * 1024;
	return maxClientBodySize;
}