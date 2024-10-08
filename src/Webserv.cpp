#include "../include/Webserv.hpp"
#include "Webserv.hpp"

PollFds					pollFdsList;

std::map<int, Connection>					connections;

std::map<int, std::vector<ServerBlock> >	serversToFd;
std::map<int, Request>						requests;
std::map<int, Response> 					responses;
bool										running = true;

static void	addNewClientAndConnection(int fd, bool isListener)
{
	struct pollfd newPollFd;

	memset(&newPollFd, 0, sizeof(newPollFd));
	newPollFd.fd = fd;
	newPollFd.events = POLLIN | POLLOUT;
	newPollFd.revents = 0;
	pollFdsList.push_back(newPollFd);

	connections[fd] = (Connection){isListener, std::time(0), Request(), Response()};
}

static int	createServerSocket(int port)
{
	int 				serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in	addr;
	int 				opt = 1;

	if (serverSocket == -1)
		return (perror("socket"), -1);
	if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
		return (perror("setsockopt"), close(serverSocket), -1);
	memset((char*)&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons(port);
	if (bind(serverSocket, (struct sockaddr *)&addr, sizeof(addr)) == -1)
		return (perror("bind"), close(serverSocket), -1);
	if (listen(serverSocket, 10) == -1)
		return (perror("listen"), close(serverSocket), -1);
	return (serverSocket);
}

static void	initiateWebServer(Configuration &config)
{
	std::vector<ServerBlock>	servers = config.getServerBlocks();
	std::set<Hostport>			hostPorts;
	std::set<int>				listenFds;

	for (std::vector<ServerBlock>::iterator it = servers.begin(); it != servers.end(); it++)
	{
		int	serverSocket;

		std::cout << "(" << it->hostPort.first << ":" << it->hostPort.second << ")"<< std::endl;
		if (hostPorts.find(it->hostPort) != hostPorts.end())
			continue;
		serverSocket = createServerSocket(it->hostPort.second);
		if (serverSocket == -1)
			continue;
		listenFds.insert(serverSocket);
		hostPorts.insert(it->hostPort);
	}
	for (std::set<int>::iterator it = listenFds.begin(); it != listenFds.end(); it++)
		addNewClientAndConnection(*it, true);
}

static void	removeClient(int fd)
{
	std::cout << "Removing client on fd: " << fd << std::endl;
	close(fd);
	connections.erase(fd);
	for (PollFds::iterator it = pollFdsList.begin(); it != pollFdsList.end(); it++)
	{
		if (it->fd == fd)
		{
			it = pollFdsList.erase(it);
			break;
		}
	}
}

static void	acceptConnection(int fd)
{
	int	clientFd = accept(fd, NULL, NULL);
	int	opt = 1;

	if (clientFd < 0)
	{
		perror("accept");
		return;
	}
	if (fcntl(clientFd, F_SETFL, O_NONBLOCK) < 0)
	{
		close(clientFd);
		perror("fcntl");
		return;
	}
	if (setsockopt(clientFd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) < 0)
	{
		close(clientFd);
		perror("setsockopt");
		return;
	}
	addNewClientAndConnection(clientFd, false);
	std::cout << "New client connected on fd: " << clientFd << std::endl;
}

static PollFds::iterator	closeConnection(PollFds::iterator it)
{
	std::cout << "Closing connection on fd: " << it->fd << std::endl;
	connections[it->fd].req.clearRequest();
	connections.erase(it->fd);
	close(it->fd);
	return pollFdsList.erase(it);
}

static int	receiveRequest(int fd)
{
	char 				buffer[BUFFER_SIZE];
	Request 			&req = connections[fd].req;
	ssize_t 			bytes;
	std::stringstream	ss;

	memset(buffer, 0, BUFFER_SIZE);
	bytes = recv(fd, buffer, BUFFER_SIZE, 0);
	if (bytes < 0)
		return FAILURE;
	buffer[bytes] = '\0';
	// std::cout << "[" << buffer << "]" << std::endl;
	std::cout << "request received" << std::endl;
	if (bytes == 0)
	{
		std::cout << "Connection closed" << std::endl;
		// closeConnection(fd);
		return FAILURE;
	}
	ss.write(buffer, bytes);
	req.parseRequest(ss);
	return SUCCESS;
}

static void	sendResponse(int fd, Configuration &config)
{
	std::string	str;

	connections[fd].res = Response(connections[fd].req);
	printRequest(connections[fd].req);
	str = connections[fd].res.getResponse(config);
	send(fd, str.c_str(), str.size(), 0);
	std::cout << "Response sent" << std::endl;
	connections[fd].req.clearRequest();
	connections[fd].startTime = std::time(0);
	connections[fd].res.clearResponse();
}

static void handleSIGINT(int sig)
{
	(void)sig;
	std::cout << BLUE << "\nSIGINT received, stopping server" << SET << std::endl;
	running = false;
}

static void	printPollFds()
{
	std::cout << "POLLFDS :" << std::endl;
	for (PollFds::iterator it = pollFdsList.begin(); it != pollFdsList.end(); it++)
	{
		std::cout
		<< "{fd: " << it->fd << ", events: " 
		<< it->events << ", revents: " 
		<< ((it->revents & POLLIN) ? "POLLIN | " : "X | ")
		<< ((it->revents & POLLOUT) ? "POLLOUT | " : "X | ")
		<< "}"
		<< std::endl;
	}
	std::cout << std::endl;
}

static void	printPfd(PollFds::iterator it)
{
	std::cout 
		<< "PFD: {fd: " << it->fd << ", events: " 
		<< it->events << ", revents: " 
		<< ((it->revents & POLLIN) ? "POLLIN | " : "X | ")
		<< ((it->revents & POLLOUT) ? "POLLOUT | " : "X | ")
		<< "}"
	<< std::endl;
}

// static void	checkTimeouts()
// {
// 	std::time_t	now = std::time(0);
// 	std::vector<int>	toClose;

// 	if (connections.size() == 0)
// 		return;
// 	for (std::map<int, Connection>::iterator it = connections.begin(); it != connections.end(); it++)
// 	{
// 		if (it->second.isListener)
// 			continue;
// 		if (now - it->second.startTime >= 3)
// 		{
// 			std::cout << "Connection timeout on fd: " << it->first << std::endl;
// 			toClose.push_back(it->first);
// 		}
// 	}
// 	for (std::vector<int>::iterator it = toClose.begin(); it != toClose.end(); it++)
// 		closeConnection(*it);
// }

void runWebServer(Configuration &config)
{
	std::string	wait[] = {"⠋", "⠙", "⠸", "⠴", "⠦", "⠇"};
	int			timeout = 1000;
	int			n = 0;

	initiateWebServer(config);
	signal(SIGINT, handleSIGINT);
	while (running)
	{
		int nfds = poll(pollFdsList.data(), pollFdsList.size(), timeout);
		if ((nfds < 0 && errno != EINTR) || running == false)
		{
			std::cout << "COUCOUCOU" << std::endl;
			break;
		}
		if (nfds >= 0)
		{
			std::cout << GREEN << "Server running " << wait[n++] << SET << "\r" << std::flush;
			if (n == 6)
				n = 0;
		}
		
		int j = 0;
		for (PollFds::iterator it = pollFdsList.begin(); it != pollFdsList.end() && j < nfds;)
		{
		    if (it->revents == 0)
		    {
		        ++it;
		        continue;
		    }
		    j++;
		    if (it->revents & POLLIN)
		    {
		        if (connections.find(it->fd) != connections.end() && connections[it->fd].isListener)
		        {
		            acceptConnection(it->fd);
		        }
		        else
		        {
		            if (receiveRequest(it->fd) == FAILURE)
		            {
		                it = closeConnection(it);  // Update the iterator properly
		                continue;  // Skip incrementing the iterator since it's already updated
		            }
		        }
		    }
		    if (it->revents & POLLOUT)
		    {
		        if (connections[it->fd].req.getParsingState() == PARSING_DONE)
		            sendResponse(it->fd, config);
		    }
		    ++it;
		}

		// checkTimeouts();
	}
	// for (PollFds::iterator it = pollFdsList.begin(); it != pollFdsList.end(); it++)
	// 	close(it->fd);
	std::cout << "Server stopped" << std::endl;
}
