#ifndef CONFIGURATION_HPP
#define CONFIGURATION_HPP

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <cstddef>
#include <sstream>
#include <cstdlib>// for std::atoi

enum http_method {
	GET,
	POST,
	DELETE,
	UNKNOWN
};

struct BodySize {
	std::string	value;
	std::string	unit;
};

struct LocationBlock {
	std::string							path;// file or directory
	std::string							root;
	std::string							alias;
	BodySize							clientMaxBodySize;
	bool								autoindex;
	std::vector<std::string>			indexes;
	std::map<std::string, std::string>	redirects;// {code, address}
	bool								pathInfo;
	std::map<std::string, std::string>	cgiParams;// {extension, file}
	std::string							uploadLocation;
	std::vector<http_method>			methods;// GET, POST, DELETE by default
};

struct ServerBlock {
	int									port;// from listen directive
	std::string							host;// from listen directive
	std::vector<std::string>			serverNames;// maybe none or more
	std::string							root;
	std::map<std::string, std::string>	errorPages;// {error code, uri}
	BodySize							clientMaxBodySize;
	std::vector<LocationBlock>			locationBlocks;
};

class Configuration
{
	private:
		// maybe remove it later
		std::string							m_configFile;

		// all the content of the config file
		std::stringstream					m_content;

		// all the server blocks parsed from the config file
		std::vector<ServerBlock>			m_serverBlocks;

		// std::vector<int>			_ports; // for vic's part
		// and more

		void		parseServerBlock(std::stringstream &content);
		void		parseLocationBlock(std::stringstream &content, ServerBlock &serverBlock, std::string const &line);

		void		parseServerDirective(std::string const &line, ServerBlock &serverBlock);
		void		parseLocationDirective(std::string &line, LocationBlock &locationBlock);

		void		setLocationValues(std::string const &key, std::string const &value, LocationBlock &locationBlock);
		void		setServerValues(std::string const &key, std::string const &value, ServerBlock &serverBlock);

		void		setListen(std::string const &value, ServerBlock &serverBlock);
		void		addErrorPage(std::string const &value, ServerBlock &serverBlock);
		void		setRedirect(std::string const &value, LocationBlock &locationBlock);
		void		setCgi(std::string const &value, LocationBlock &locationBlock);
		void		setMethod(std::string const &value, LocationBlock &locationBlock);

	public:
		// std::vector<ServerBlock>			m_serverBlocks;
		Configuration();
		Configuration(std::string const &t_configFile);
		~Configuration();
		// std::vector<int>			getPorts() const;


		// getters (We need getter for server_names, ports, etc)
		std::vector<ServerBlock> const &getServerBlocks() const;
		int	getClientMaxBodySize(BodySize const &bodySize);
		//...
		
		// tools
		const int			getBodySize(BodySize const &bodySize) const;
		std::vector<int>	getPorts() const;
		void				printConfig() const;
		//...
};

#endif // CONFIGURATION_HPP
