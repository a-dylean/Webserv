#pragma once

#include "Request.hpp"
#include "Response.hpp"
#include "Configuration.hpp"

#include <utility>
#include <algorithm>
#include <iostream>
#include <string>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <vector>
#include <map>
#include <set>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <poll.h>

#define MAX_CLIENTS 32
#define BUFFER_SIZE 1024

//functions
void setNonBlocking(int fd);
void setOpt(int fd);
int createSocket(ServerBlock serverBlock, struct sockaddr_in servaddr);
void listenToSockets();
void setPollWatchlist(int fd);
void initiateWebServer(const Configuration &config);
void acceptConnection(int fd);
void receiveRequest(int fd);
void sendResponse(int fd, std::string response);
void runWebserver(Configuration &config);