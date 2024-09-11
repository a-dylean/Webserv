#pragma once

#include <bits/stdc++.h> // for std::stringstream
#include <fstream>
#include <vector>
#include <map>
#include <cstddef>
#include <iostream>
#include <string>
#include <sstream>
#include <cstdlib> // for std::atoi

#define CRLF "\r\n" // carriage return line feed
#define LF '\n'
#define CR '\r'

enum http_method
{
	GET,
	POST,
	DELETE,
	UNKNOWN
};

struct BodySize
{
	std::string value;
	std::string unit;
};

struct LocationBlock {
	std::string							path;// file or directory
	std::string							root;
	std::string							alias;
	BodySize							clientMaxBodySize;
	int									bodySize;
	bool								autoindex;
	std::vector<std::string>			indexes;
	bool                               	redirection;
	std::map<std::string, std::string>	redirects;// {code, address}
	bool								pathInfo;
	std::map<std::string, std::string>	cgiParams;// {extension, file}
	std::string							uploadLocation;
	std::map<std::string, std::string>	errorPages;
	std::vector<http_method>			methods;// GET, POST, DELETE by default
};

struct ServerBlock {
	int									port;// from listen directive
	std::string							host;// from listen directive
	std::vector<std::string>			serverNames;// maybe none or more
	std::string							root;
	std::map<std::string, std::string>	errorPages;// {error code, uri}
	BodySize							clientMaxBodySize;
	int									bodySize;
	std::vector<LocationBlock>			locationBlocks;
};

class Configuration
{
private:
	// maybe remove it later
	std::string m_configFile;

	// all the content of the config file
	std::stringstream m_content;

	// all the server blocks parsed from the config file
	std::vector<ServerBlock> m_serverBlocks;

	// std::vector<int>			_ports; // for vic's part
	// and more

		void		parseServerBlock(std::string const &line);
		void		parseLocationBlock(ServerBlock &serverBlock, std::string const &line);
		
		void		parseServerDirective(std::string const &line, ServerBlock &serverBlock);
		void		parseLocationDirective(std::string &line, LocationBlock &locationBlock);
		
		void		setLocationValues(std::string const &key, std::string const &value, LocationBlock &locationBlock);
		void		setServerValues(std::string const &key, std::string const &value, ServerBlock &serverBlock);

	public:
		// std::vector<ServerBlock>			m_serverBlocks;
		Configuration();
		Configuration(std::string const &t_configFile);
		~Configuration();
		// std::vector<int>			getPorts() const;


		// getters (We need getter for server_names, ports, etc)
		std::vector<ServerBlock> const &getServerBlocks() const;
		//...
		
		// tools
		static const int	getBodySize(BodySize const &bodySize);
		std::vector<int>	getPorts() const;
		void				printConfig() const;
		//...
};
