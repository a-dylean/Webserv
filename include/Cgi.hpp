#pragma once
#include "Configuration.hpp"
#include "Request.hpp"
#include "Response.hpp"

#include <sys/types.h>
#include <sys/wait.h>
#include <ctime>
#include <sys/select.h>
#include <signal.h>

#define CGITIMEOUT 2

class Response;

void handleCGI(LocationBlock &location, Request &req, Response &res);
bool needsCGI(LocationBlock location, Request &req);
