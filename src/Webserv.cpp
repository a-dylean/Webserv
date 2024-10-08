#include "../include/Webserv.hpp"

std::map<std::pair<std::string, int>, int> socketsToPorts;
std::map<int, std::vector<ServerBlock> > serversToFd;
std::vector<int> listenFds;
std::vector<struct pollfd> pollFdsList;
std::map<int, Request> requests;
std::map<int, Response> responses;

bool run;

void setNonBlocking(int fd)
{
	if (fcntl(fd, F_SETFL, O_NONBLOCK) < 0)
		throw std::runtime_error("fcntl() failed");
}

void setOpt(int fd)
{
	int opt = 1;
	if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) < 0)
		throw std::runtime_error("setsockopt() failed");
}

int createSocket(ServerBlock serverBlock, struct sockaddr_in servaddr)
{
	int socket_fd;
	socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_fd < 0)
		throw std::runtime_error("socket() failed");
	setNonBlocking(socket_fd);
	setOpt(socket_fd);
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = INADDR_ANY;
	servaddr.sin_port = htons(serverBlock.port);
	return socket_fd;
}

void listenToSockets()
{
	for (std::map<int, std::vector<ServerBlock> >::iterator it = serversToFd.begin(); it != serversToFd.end(); it++)
	{
		int socket_fd = it->first;
		std::vector<ServerBlock> serverBlocks = it->second;
		listenFds.push_back(socket_fd);
	}
}

void rmFromPollWatchlist(int fd)
{
	for (size_t i = 0; i < pollFdsList.size(); i++)
	{
		if (pollFdsList[i].fd == fd)
		{
			close(fd);
			pollFdsList.erase(pollFdsList.begin() + i);
			break;
		}
	}
}

void setPollWatchlist(int fd)
{
	struct pollfd pfd = (struct pollfd){fd, POLLIN | POLLOUT, 0};
	pollFdsList.push_back(pfd);
}

void initiateWebServer(const Configuration &config)
{
	std::vector<ServerBlock> serverBlocks = config.getServerBlocks();
	for (size_t i = 0; i < serverBlocks.size(); i++)
	{
		std::pair<std::string, int> ipPort = std::make_pair(serverBlocks[i].host, serverBlocks[i].port);
		if (socketsToPorts.count(ipPort) == 0)
		{
			struct sockaddr_in servaddr;
			servaddr.sin_family = AF_INET;
			servaddr.sin_addr.s_addr = INADDR_ANY;
			servaddr.sin_port = htons(serverBlocks[i].port);
			int socket_fd = createSocket(serverBlocks[i], servaddr);
			setNonBlocking(socket_fd);
			if (bind(socket_fd, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
				throw std::runtime_error("bind() failed");
			if (listen(socket_fd, MAX_CLIENTS) < 0)
				throw std::runtime_error("listen() failed");
			socketsToPorts[ipPort] = socket_fd;
			serversToFd[socket_fd].push_back(serverBlocks[i]);
		}
		else
		{
			int socket_fd = socketsToPorts[ipPort];
			serversToFd[socket_fd].push_back(serverBlocks[i]);
		}
	}
	listenToSockets();
	for (size_t i = 0; i < listenFds.size(); i++)
	{
		setPollWatchlist(listenFds[i]);
	}
}

void acceptConnection(int fd)
{
	struct sockaddr_in cli;
	socklen_t len = sizeof(cli);
	int new_connection = accept(fd, (struct sockaddr *)&cli, &len);
	if (new_connection < 0)
		throw std::runtime_error("accept() failed");
	setNonBlocking(new_connection);
	setOpt(new_connection);
	setPollWatchlist(new_connection);
	serversToFd[new_connection] = serversToFd[fd];
	requests[new_connection] = Request();
}

void receiveRequest(int fd)
{
	char buffer[BUFFER_SIZE];
	std::string fullRequest;
	int ret_total = 0;

	ssize_t return_value = recv(fd, buffer, BUFFER_SIZE, 0);
	if (return_value == -1)
		return;
	
	Request &req = requests[fd];
	if (return_value == 0)
	{
		req.setRequestState(RECEIVED);
		return;
	}
	std::stringstream ss;
	ss.write(buffer, return_value);
	req.parseRequest(ss);
}

void sendResponse(int fd, Configuration &config)
{
	printRequest(requests[fd]);
	Response resp(requests[fd]);
	responses[fd] = resp;
	std::string generatedResponse = responses[fd].getResponse(config);
	int bytes_sent = send(fd, generatedResponse.c_str(), generatedResponse.size(), 0);
	requests[fd].setRequestState(PROCESSED);
}

int findCount(int fd)
{
	int count = 0;
	for (size_t i = 0; i < listenFds.size(); i++)
	{
		if (listenFds[i] == fd)
			count++;
	}
	return count;
}

void runWebserver(Configuration &config)
{
	int timeout = 1000;

	run = true;
	while (run)
	{
		int nfds = poll(&pollFdsList[0], pollFdsList.size(), timeout);
		if (nfds < 0 && errno == EINTR)
			break;
		else if (nfds < 0)
			throw std::runtime_error("poll() failed");
		else
		if (nfds == 0)
		{
			std::cout << "Waiting for connection or request..." << (run ? " (still running)" : " (shutting down)") << std::endl;
		}
		//The variable j serves as a counter to keep track of the number of file 
		//descriptors that have events (revents) set. This is necessary because 
		//the poll function returns the number of file descriptors with events, 
		//and the loop needs to process exactly that many file descriptors.
		int j = 0;
		for (size_t i = 0; i < pollFdsList.size() && j < nfds; i++)
		{
			int fd = pollFdsList[i].fd;

			if (pollFdsList[i].revents == 0)
				continue;
			j++;
			if (pollFdsList[i].revents & POLLIN)
			{
				if (findCount(fd) == 1)
				{
					acceptConnection(fd);
				}
				else
				{
					receiveRequest(fd);
				}
			}
			else if (pollFdsList[i].revents & POLLOUT)
			{
				if (requests[fd].getParsingState() == PARSING_DONE)
				{
					sendResponse(fd, config);
				}
			}
			if (requests[fd].getRequestState() == PROCESSED)
			{
				rmFromPollWatchlist(fd);
				serversToFd.erase(fd);
				requests[fd].clearRequest(); // check if it's required
				requests.erase(fd);
				// responses[fd].clear();
				// responses.erase(fd);
				close(fd);
			}
		}
	}
	std::cout << "Server shutting down..." << std::endl;
}
