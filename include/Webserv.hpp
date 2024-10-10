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
#include <ctime>

#define MAX_CLIENTS 42
#define MAX_EVENTS 1024
#define BUFFER_SIZE 1024
#define TIMEOUT 300// 5 minutes

#define GREEN "\033[32m"
#define BLUE "\033[34m"
#define SET "\033[0m"

#define FAILURE -1
#define SUCCESS 0

typedef std::vector<ServerBlock> Servers;
typedef std::vector<struct pollfd> PollFds;

struct Connection
{
	int				fd;
	bool			isListener;
	std::time_t		startTime;
	Request			req;
	Response		res;
	bool			isActive;
};

void    runWebServer(Configuration &config);