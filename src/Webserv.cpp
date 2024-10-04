#include "../include/Webserv.hpp"
#include "Webserv.hpp"

std::map<int, std::time_t> startTimeForFd;


std::map<std::pair<std::string, int>, int>	socketsToPorts;// unused in this code...
std::map<int, std::vector<ServerBlock> >	serversToFd;
std::map<int, std::time_t>					fdToTimeoutCheck;
std::vector<int>							listenFds;
std::vector<struct pollfd>					pollFdsList;
std::map<int, Request>						requests;
std::map<int, Response> 					responses;
bool										running = true;

// static void printPollFdsList(std::vector<struct pollfd> &pollFdsList)
// {
// 	std::cout << "PollFdsList ( " << pollFdsList.size() << " fds):" << std::endl;
// 	for (std::vector<struct pollfd>::iterator it = pollFdsList.begin(); it != pollFdsList.end(); it++)
// 	{
// 		std::cout << "fd: " << it->fd << " events: " << it->events << " revents: " << it->revents << std::endl;
// 	}
// }

// static void	printListenFds()
// {
// 	std::cout << "ListenFds (" << listenFds.size() << "):" << std::endl;
// 	for (std::vector<int>::iterator it = listenFds.begin(); it != listenFds.end(); it++)
// 	{
// 		std::cout << *it << std::endl;
// 	}
// }

// static void	printSocketsToPorts()
// {
// 	std::cout << "SocketsToPorts (" << socketsToPorts.size() << "):" << std::endl;
// 	for (std::map<std::pair<std::string, int>, int>::iterator it = socketsToPorts.begin(); it != socketsToPorts.end(); it++)
// 	{
// 		std::cout << it->first.first << ":" << it->first.second << "(host:port) -> socket: " << it->second << std::endl;
// 	}
// }

// static void	printServersToFd()
// {
// 	std::cout << "ServersToFd (" << serversToFd.size() << "):" << std::endl;
// 	for (std::map<int, std::vector<ServerBlock> >::iterator it = serversToFd.begin(); it != serversToFd.end(); it++)
// 	{
// 		ServerBlock	server = it->second[0];
// 		std::cout << "fd: " << it->first << " server port: " << server.port << std::endl;
// 	}
// }

void rmFromPollWatchlist(int fd)
{
	for (std::vector<struct pollfd>::iterator it = pollFdsList.begin(); it != pollFdsList.end(); it++)
	{
		if (it->fd == fd)
		{
			pollFdsList.erase(it);
			break;
		}
	}
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

void	initiateWebServer(Configuration &config)
{
	std::vector<ServerBlock>	servers = config.getServerBlocks();

	for (std::vector<ServerBlock>::iterator it = servers.begin(); it != servers.end(); it++)
	{
		std::pair<std::string, int>	hostPort = std::make_pair(it->host, it->port);
		int					 		serverSocket;

		// std::cout << "(" << it->host << ":" << it->port << ")"<< std::endl;
		if (socketsToPorts.find(hostPort) != socketsToPorts.end())
			serverSocket = socketsToPorts[hostPort];
		else
		{
			serverSocket = createServerSocket(it->port);
			if (serverSocket == -1)
				continue;
			socketsToPorts[hostPort] = serverSocket;
		}
		serversToFd[serverSocket].push_back(*it);
	}
	// printServersToFd();
	// printSocketsToPorts();
	for (std::map<int, std::vector<ServerBlock> >::iterator it = serversToFd.begin(); it != serversToFd.end(); it++)
		listenFds.push_back(it->first);
	// printListenFds();
	for (std::vector<int>::iterator it = listenFds.begin(); it != listenFds.end(); it++)
	{
		struct pollfd pfd;
		pfd.fd = *it;
		pfd.events = POLLIN | POLLOUT | POLLHUP;
		pfd.revents = 0;
		pollFdsList.push_back(pfd);
	}
	// printPollFdsList(pollFdsList);
	requests.clear();
	responses.clear();
}

void	acceptConnection(int fd)
{
	int	newConnection = accept(fd, NULL, NULL);
	int	opt = 1;
	
	if (newConnection < 0)
	{
		perror("accept");
		return;
	}
	if (fcntl(newConnection, F_SETFL, O_NONBLOCK) < 0)
	{
		close(newConnection);
		perror("fcntl");
		return;
	}
	if (setsockopt(newConnection, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) < 0)
	{
		close(newConnection);
		perror("setsockopt");
		return;
	}
	struct pollfd	pfd = (struct pollfd){newConnection, POLLIN | POLLOUT | POLLHUP, 0};
	pollFdsList.push_back(pfd);
	serversToFd[newConnection] = serversToFd[fd];
	requests[newConnection] = Request();
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
	// printRequest(requests[fd]);
	static int i = 0;
	// std::cout << "Sending response (" << i++ << ")" << std::endl;
	Response resp(requests[fd]);
	responses[fd] = resp;
	startTimeForFd[fd] = std::time(0);
	std::string generatedResponse = responses[fd].getResponse(config);
	int bytes_sent = send(fd, generatedResponse.c_str(), generatedResponse.size(), 0);
	if (bytes_sent == -1)
	{
		std::cout << "ERROR SEND" << std::endl;
		perror("send");
		return;
	}
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

static void handleSIGINT(int sig)
{
	(void)sig;
	std::cout << BLUE << "\n  { SIGINT received, stopping server }" << SET << std::endl;
	running = false;
}

static void	printRequests(std::map<int, Request> &requests)
{
	std::cout << "Requests:" << std::endl;
	for (std::map<int, Request>::iterator it = requests.begin(); it != requests.end(); it++)
	{
		std::cout << "fd: " << it->first << " state: " << it->second.getRequestState() << std::endl;
	}
}

void checkTimeouts(std::map<int, std::time_t> &startTimeForFd)
{
	std::time_t currentTime = std::time(0);
	std::map<int, std::time_t>::iterator it = startTimeForFd.begin();
	while (it != startTimeForFd.end())
	{
		if (currentTime - it->second > 3)
		{
			rmFromPollWatchlist(it->first);
			serversToFd.erase(it->first);
			requests[it->first].clearRequest();
			requests.erase(it->first);
			close(it->first);
			startTimeForFd.erase(it++);
		}
		else
			it++;
	}
}

void runWebServer(Configuration &config)
{
	const std::string	wait[] = {"⠋", "⠙", "⠸", "⠴", "⠦", "⠇"};
	int					n = 0;

	initiateWebServer(config);
	signal(SIGINT, handleSIGINT);
	while (running)
	{
		int nfds = poll(&pollFdsList[0], pollFdsList.size(), TIMEOUT);
		if (nfds < 0)
			break;
		if (nfds == 0)
		{
			// std::cout << GREEN << "  { Waiting for connection " << wait[n++ % 6] << " }" << SET << "\r" << std::flush;
			std::cout << GREEN << "Waiting for connection..." << SET << std::endl;
			// if (n == 6)
			// 	n = 0;
		}
		// else if (nfds > 0)
		// 	std::cout << GREEN << "Connection received!" << SET << std::endl;

		int j = 0;
		int pollFdsListSize = pollFdsList.size();
		for (int i = 0; i < pollFdsListSize && j < nfds; i++)
		{
			struct pollfd	pfd = pollFdsList[i];

			if (pfd.revents == 0 || pfd.fd == -1)
				continue;
			j++;
			if (pfd.revents & POLLIN)
			{
				if (findCount(pfd.fd) > 0)
					acceptConnection(pfd.fd);
				else
					receiveRequest(pfd.fd);
			}
			if (requests[pfd.fd].getParsingState() == PARSING_DONE && pfd.revents & POLLOUT)
				sendResponse(pfd.fd, config);
			if (requests[pfd.fd].getRequestState() == PROCESSED)
			{
				requests[pfd.fd].clearRequest();
				requests.erase(pfd.fd);
			}
			if (pfd.revents & POLLHUP)
			{
				rmFromPollWatchlist(pfd.fd);
				serversToFd.erase(pfd.fd);
				requests[pfd.fd].clearRequest();
				requests.erase(pfd.fd);
				close(pfd.fd);
			}
			checkTimeouts(startTimeForFd);
		}
	}
	for(std::map<int, std::vector<ServerBlock> >::iterator it = serversToFd.begin(); it != serversToFd.end(); it++)
		close(it->first);
}



// #include "../include/Webserv.hpp"
// #include "Webserv.hpp"

// std::vector<int>                            listeners;
// std::map<int, Request>                      requests;
// std::map<int, Response>                     responses;
// bool                                        running = true;
// std::vector<struct pollfd>                  pfds;

// static void handleSIGINT(int sig)
// {
//     (void)sig;
//     std::cout << BLUE << "\n  { SIGINT received, stopping server }" << SET << std::endl;
//     running = false;
// }

// // Créer une socket de serveur sur un port donné
// static int create_server_socket(int port, std::string host)
// {
//     int                 socket_fd;
//     int                 status;
//     struct sockaddr_in  sa;

// 	(void)host;
//     // Préparation de l'adresse et du port pour la socket de notre serveur
//     memset(&sa, 0, sizeof(sa));
//     sa.sin_family = AF_INET;
//     sa.sin_addr.s_addr = htonl(INADDR_ANY);
//     sa.sin_port = htons(port);

//     // Création de la socket
//     socket_fd = socket(sa.sin_family, SOCK_STREAM, 0);
//     if (socket_fd == -1) {
//         perror("create_server_socket: socket");
//         return -1;
//     }
//     std::cout << "Created server socket on port " << port << ", fd: " << socket_fd << std::endl;

//     // Liaison de la socket à l'adresse et au port
//     status = bind(socket_fd, (struct sockaddr *)&sa, sizeof sa);
//     if (status != 0) {
//         perror("create_server_socket: bind");
//         close(socket_fd);
//         return -1;
//     }
//     std::cout << "Bound server socket to port " << port << std::endl;

//     status = listen(socket_fd, MAX_CLIENTS);
//     if (status != 0) {
//         perror("server_loop: listen");
//         close(socket_fd);
//         return -1;
//     }

//     return (socket_fd);
// }

// // Fonction pour accepter une nouvelle connexion
// static void acceptConnection(int server_socket)
// {
//     int             client_fd;
//     struct pollfd   pfd;

//     client_fd = accept(server_socket, NULL, NULL);
//     if (client_fd == -1) {
//         perror("acceptConnection: accept");
//         return;
//     }

//     if (fcntl(client_fd, F_SETFL, O_NONBLOCK) == -1) {
//         perror("acceptConnection: fcntl");
//         close(client_fd);
//         return;
//     }

//     pfd.fd = client_fd;
//     pfd.events = POLLIN | POLLOUT; // Always listen for both read and write events

//     pfds.push_back(pfd);
//     std::cout << "Accepted new connection on client socket " << client_fd << std::endl;
//     requests[client_fd] = Request();
//     responses[client_fd] = Response();
// }

// // Fonction pour lire les données d'une socket client
// static void receiveRequest(int i, int client_fd) {
//     char    buffer[BUFSIZ];
//     Request &req = requests[client_fd];
//     int     bytes_read;

// 	(void)i;
//     bytes_read = recv(client_fd, buffer, BUFSIZ, 0);
//     if (bytes_read == -1) {
//         perror("receiveRequest: recv");
//         return;
//     } else if (bytes_read == 0) {
//         req.setRequestState(RECEIVED);
//         return;
//     }

//     std::stringstream ss;
//     ss.write(buffer, bytes_read);
//     req.parseRequest(ss);
// }

// static void sendResponse(int i, Configuration &config) {
//     int receiver_fd = pfds[i].fd;

//     if (requests[receiver_fd].getRequestState() != RECEIVED)
//         return;

//     std::string generatedResponse = responses[receiver_fd].getResponse(config);
//     int bytes_sent = send(receiver_fd, generatedResponse.c_str(), generatedResponse.size(), 0);
//     if (bytes_sent == -1) {
//         perror("sendResponse: send");
//         close(receiver_fd);
//         pfds.erase(pfds.begin() + i);
//         return;
//     }

//     requests[receiver_fd].setRequestState(PROCESSED);

//     close(receiver_fd);
//     pfds.erase(pfds.begin() + i);
// }

// // Fonction principale du serveur pour écouter sur plusieurs ports
// void server_loop(const Configuration &config)
// {
//     std::vector<ServerBlock> servers = config.getServerBlocks();
//     int status;

//     // Création de sockets pour chaque port
//     for (std::vector<ServerBlock>::const_iterator it = servers.begin(); it != servers.end(); ++it)
//     {
//         int server_socket = create_server_socket(it->port, it->host);
//         if (server_socket == -1)
//             continue;

//         // Ajoute la socket du serveur à l'ensemble des sockets surveillées
//         struct pollfd server_pollfd;
//         server_pollfd.fd = server_socket;
//         server_pollfd.events = POLLIN; // Serveur socket seulement en mode lecture
//         pfds.push_back(server_pollfd);

//         listeners.push_back(server_socket);
//     }

//     signal(SIGINT, handleSIGINT);
//     while (running) {
//         int nfds = pfds.size();
//         status = poll(pfds.data(), nfds, 1000);
//         if (status == -1) {
//             perror("poll");
//             break;
//         } else if (status == 0) {
//             continue;
//         }

//         for (int i = 0; i < nfds; ++i) {
//             if (pfds[i].revents == 0)
//                 continue;

//             if (pfds[i].revents & POLLIN) {
//                 if (std::find(listeners.begin(), listeners.end(), pfds[i].fd) != listeners.end()) {
//                     acceptConnection(pfds[i].fd);
//                 } else {
//                     receiveRequest(i, pfds[i].fd);
//                 }
//             }

//             if (pfds[i].revents & POLLOUT) {
//                 sendResponse(i, (Configuration &)config);
//             }

//             if (pfds[i].revents & POLLHUP) {
//                 close(pfds[i].fd);
//                 pfds.erase(pfds.begin() + i);
//                 --i;
//             }
//         }
//     }
// }
