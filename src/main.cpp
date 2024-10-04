#include "../include/Webserv.hpp"

int	main(int ac, char **av)
{
	std::string	defaultConfigPath = "./config/default.conf";

	if (ac == 2)
		defaultConfigPath = av[1];
	try
	{
		Configuration	config(defaultConfigPath);
		
		runWebServer(config);
		// server_loop(config);
	}
	catch (std::exception &e) {
		std::cerr << e.what() << std::endl;
	}
	return (0);
}
