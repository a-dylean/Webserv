#include "../include/Webserv.hpp"

std::map<std::pair<std::string, int>, int> socketsToPorts;
std::map<int, std::vector<ServerBlock> > serversToFd;
std::vector<int> listenFds;
std::vector<struct pollfd> pollFdsList;
std::map<int, Request> requests;

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
			close(pollFdsList[i].fd);
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
	std::vector<ServerBlock> serverBlocks = config.m_serverBlocks;

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
}

void receiveRequest(int fd)
{
	int return_value = 0;
	char buffer[BUFFER_SIZE];
	std::string fullRequest;
	int ret_total = 0;

	while ((return_value = recv(fd, buffer, BUFFER_SIZE - 1, 0)) > 0)
	{
		fullRequest.append(buffer, return_value);
		ret_total += return_value;
	}
	Request req(fullRequest);
	req.printRequest(req);
	req.setRequestState(RECEIVED);
	requests.insert(std::make_pair(fd, req));
}

void sendResponse(int fd)
{
	std::string response = "HTTP/1.1 200 OK\n";
	response += "Content-Type: text/html\r\n";
	response += "Content-Length: 13\n\n";
	response += "Hello World !\r\n\r\n";
	int bytes_sent = send(fd, response.c_str(), response.size(), 0);
	requests[fd].setRequestState(SENT);
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

void runWebserver(void)
{
	int timeout = 500;

	while (1)
	{
		int nfds = poll(&pollFdsList[0], pollFdsList.size(), timeout);
		if (nfds < 0)
			throw std::runtime_error("poll() failed");
		if (nfds == 0)
		{
			std::cout << "Waiting for connection" << std::endl;
		}
		for (size_t i = 0; i < pollFdsList.size(); i++)
		{
			int fd = pollFdsList[i].fd;

			if (pollFdsList[i].revents == 0)
				continue;
			if (pollFdsList[i].revents & POLLIN)
			{
				if (findCount(fd) == 1)
				{
					std::cout << "Accepting connection for" << fd << std::endl;
					acceptConnection(fd);
				}
				else
				{
					receiveRequest(fd);
				}
				std::cout << "fd count: " << findCount(fd) << std::endl;
			}
			if (pollFdsList[i].revents & POLLOUT)
			{
				std::cout << "Sending response" << std::endl;
				sendResponse(fd);
			}
			if (requests[fd].getRequestState() == SENT)
			{
				rmFromPollWatchlist(fd);
				serversToFd.erase(fd);
				requests.erase(fd);
				// responses[fd].clear();
				// responses.erase(fd);
				close(fd);
			}
		}
	}
}