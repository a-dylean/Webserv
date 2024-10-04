#ifndef WEBSERV_HPP
#define WEBSERV_HPP

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

#include <stdlib.h>




#include <cerrno>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <poll.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>





#define MAX_CLIENTS 32
#define BUFFER_SIZE 1024
#define TIMEOUT 500

#define GREEN "\033[32m"
#define BLUE "\033[34m"
#define SET "\033[0m"

extern int  uploadNb;

void    runWebServer(Configuration &config);

void server_loop(const Configuration &config);

#endif // WEBSERV_HPP