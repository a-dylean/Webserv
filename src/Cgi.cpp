#include "Cgi.hpp"

char **createEnv(Request &req, LocationBlock &location)
{
    char **env = new char *[6];
    std::stringstream ss;
    ss << "CONTENT_LENGTH=" << req.getBody().size();
    env[0] = strdup(ss.str().c_str());
    ss.str("");
    ss << "CONTENT_TYPE=" << req.getHeaders()["Content-Type"];
    env[1] = strdup(ss.str().c_str());
    ss.str("");
    ss << "UPLOAD_LOCATION=" << location.uploadLocation;
    env[2] = strdup(ss.str().c_str());
    ss.str("");
    ss << "REQUEST_METHOD=" << req.getMethod();
    env[3] = strdup(ss.str().c_str());
    ss.str("");
    ss << "QUERY_STRING=" << req.getBody() << CRLF;
    env[4] = strdup(ss.str().c_str());
    env[5] = NULL;
    return env;
}

void handleCGI(Configuration &Config, LocationBlock &location, Request &req, Response &res)
{
    char **env = createEnv(req, location);
    //should be taken from config
    std::string cgiPathWithArgs = "./www/cgi-bin/test.py";
    std::stringstream cgiOutput;
    std::cout << "CGI PATH: " << cgiPathWithArgs << std::endl;
    int pipefd[2];
    if (pipe(pipefd) == -1)
    {
        std::cerr << "pipe failed" << std::endl;
        res.setStatusCode(500);
        return;
    }
    int pid = fork();
    if (pid == -1)
    {
        std::cerr << "fork failed" << std::endl;
        res.setStatusCode(500);
        return;
    }
    else if (pid == 0)
    {
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        dup2(pipefd[1], STDERR_FILENO);
        close(pipefd[1]);
        char *args[] = {strdup(cgiPathWithArgs.c_str()), NULL};
        execve(cgiPathWithArgs.c_str(), args, env);
        exit(1);
    }
    else
    {
        close(pipefd[1]);
        char buffer[128];
        ssize_t bytesRead;
        while ((bytesRead = read(pipefd[0], buffer, sizeof(buffer) - 1)) > 0)
        {
            buffer[bytesRead] = '\0';
            cgiOutput << buffer;
        }
        close(pipefd[0]);

        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status))
        {
            std::cout << "child exited with status: " << WEXITSTATUS(status) << std::endl;
        }
        res.setBody(cgiOutput.str());
        res.setMimeType("html");
        res.setStatusCode(200);
    }
    for (int i = 0; env[i]; i++)
    {
        free(env[i]);
    }
    delete[] env;
}