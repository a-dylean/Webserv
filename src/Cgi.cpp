#include "Cgi.hpp"

//path info c'est tout ce qui est apres le premier slash
//ajouter un check de timeout de notre execve pour verifier qu'on est pas coinces dans une boucle
//infinie

std::time_t cgiStart = 0;


std::string getPathInfo(const std::string &uri, const std::string &keyword) {
    size_t pos = uri.find(keyword);
    
    if (pos != std::string::npos) 
    {
        std::string result = uri.substr(pos + keyword.length());
        if (result.empty())
            return ("");
        if (result[0] != '/')
            result = "error";
        else if(!result.empty()) 
            result = "/" + result;
        return result;
    }
    else
        return "";
}

char **createEnv(Request &req, LocationBlock &location)
{
    //let's parse the request and create the env variables
    std::string scriptName;
    std::string pathInfo;
    bool varSSet = false;
    char **env = new char *[8];
    std::stringstream ss;
    std::string uri = req.getUri();

    
    if (uri == "/submit_comment")
    {
        // std::cout << "URIHERETEST: " << uri << std::endl;
        // scriptName = location.cgiParams[".py"];
        std::string keyword = uri;
        if (location.pathInfo == true)
            pathInfo = getPathInfo(uri, keyword);
        else
            pathInfo = "";
        varSSet = true;
    }
    else if (location.pathInfo == true && varSSet == false)
    {
        // std::cout << "URIHERE: " << uri << std::endl;
        size_t pos = uri.find(location.cgiParams.begin()->first);
        if (pos != std::string::npos) //if the keyword is found
        {
            //todo faire en sorte que path info ait toujours le format "/.../.../.../"
            std::string pathInfo = getPathInfo(uri, location.cgiParams.begin()->first);          
            scriptName = uri.substr(0, pos + 3); // +3 to include ".py"
            if (pathInfo == "error")
            {
                // std::cout << "PATH INFO si on et erreur :" << pathInfo << std::endl;
                scriptName = "error";
                pathInfo = "";
            }
            // std::cout << "PATH INFO si on et tout ok :" << pathInfo << std::endl;
        }
        else
        {
            scriptName = "error";
            pathInfo = "";
            // std::cout << "PATH INFO si on et erreur :" << pathInfo << std::endl;
        }
    }
    else
    {
        scriptName = req.getUri();
        // std::cout << "SCRIPTNAME: " << scriptName << std::endl;
        pathInfo = "";
    }
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
    ss << "QUERY_STRING=" << req.getBody();
    env[4] = strdup(ss.str().c_str());
    ss.str("");
    ss << "SCRIPT_NAME=" << scriptName;
    env[5] = strdup(ss.str().c_str());
    ss.str("");
    ss << "PATH_INFO=" << pathInfo << CRLF;
    env[6] = strdup(ss.str().c_str());
    env[7] = NULL;

    return env;
}

std::string getCGIPath(char **env, LocationBlock &location)
{
    if (!env)
        return "";

    std::string tempScriptName = env[5];
    if (tempScriptName == "error")
        return (tempScriptName);

    std::string path;
    std::stringstream ss;
    std::string scriptName;
    std::string temp;

    ss << env[5];
    temp = ss.str();
    std::size_t pos = temp.find("=");
    if (pos != std::string::npos)
        scriptName = temp.substr(pos + 1);
    if (scriptName == location.cgiParams[".py"] && scriptName != "")
        return (scriptName);
    if (location.path == "/")
        path = location.root + scriptName;
    else
        path = location.root + location.path + scriptName;
    return (path);
}

static void freeEnv(char **env)
{
    for (int i = 0; env[i]; i++)
    {
        free(env[i]);
    }
    delete[] env;
}

void    handleCGI(Configuration &Config, LocationBlock &location, Request &req, Response &res)
{
    (void)Config;
    char **env = createEnv(req, location);
    if (!env)
    {
        res.setStatusCode(500);
        return;
    }
    std::stringstream   cgiOutput;
    std::string         cgiPathWithArgs = getCGIPath(env, location);
    struct stat         st;
    
    // std::cout << "CGI PATH: " << cgiPathWithArgs << std::endl;
    std::cout << "CGI PATH: " << cgiPathWithArgs << std::endl;
    if (cgiPathWithArgs == "")
    {
        std::cerr << "error in getting cgi path" << std::endl;
        res.setStatusCode(400); //?? is this the right error code?
        freeEnv(env);
        return;
    }
    else if (cgiPathWithArgs == "error")
    {
        res.setStatusCode(404);
        freeEnv(env);
        return ;
    }
    // std::cout << "CGI PATH: " << cgiPathWithArgs << std::endl;
    if (stat(cgiPathWithArgs.c_str(), &st) != 0)
    {
        std::cerr << "file does not exist" << std::endl;
        res.setStatusCode(404);
        freeEnv(env);
        return;
    }
    else if (isDirectory(cgiPathWithArgs))
    {
        std::cerr << "file is a directory" << std::endl;
        res.setStatusCode(404);
        freeEnv(env);
        return;
    }
    else
    {
        if (!(st.st_mode & S_IXUSR))
        {
            std::cerr << "file is not executable" << std::endl;
            res.setStatusCode(403);
            freeEnv(env);
            return;
        }
    }
    int pipefd[2];
    if (pipe(pipefd) == -1)
    {
        std::cerr << "pipe failed" << std::endl;
        res.setStatusCode(500);
        freeEnv(env);
        return;
    }
    int pid = fork();
    if (pid == -1)
    {
        std::cerr << "fork failed" << std::endl;
        res.setStatusCode(500);
        freeEnv(env);
        return;
    }
    else if (pid == 0)
    {
        // std::cout << "COUCOU" << std::endl;
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);
        char *args[] = {strdup(cgiPathWithArgs.c_str()), NULL};
        execve(cgiPathWithArgs.c_str(), args, env);
        perror("execve failed");
        freeEnv(env);
        exit(1);
    }
    else
    {
        close(pipefd[1]);
        char buffer[128];
        int status;
        cgiStart = std::time(0);

        while (true) 
        {
            // Check if the timeout has been exceeded
            if (std::time(0) - cgiStart >= CGITIMEOUT)
            {
                std::cout << "std::time(0) - cgiStart = " << std::time(0) - cgiStart << std::endl;
                kill(pid, SIGKILL);
                res.setStatusCode(504);
                freeEnv(env);
                close(pipefd[0]);
                res.fdToClose = true;
                return;
            }
            // Check if the child process has terminated
            if (waitpid(pid, &status, WNOHANG) != 0) 
                break; // Sleep for 10 milliseconds
        }

        if (WIFEXITED(status))
            std::cout << "child exited with status: " << WEXITSTATUS(status) << std::endl;
        ssize_t bytesRead;
        while ((bytesRead = read(pipefd[0], buffer, sizeof(buffer) - 1)) > 0)
        {
            buffer[bytesRead] = '\0';
            cgiOutput << buffer;
        }
        close(pipefd[0]);
        if (cgiOutput.str().empty() || WEXITSTATUS(status) != 0)
        {
            res.setStatusCode(500);
            freeEnv(env);
            return;
        }
        //add check here to verify that the file (cgiOutput) is finite (no infinite loops in py script)
        else
        {
            std::string output = cgiOutput.str();
            size_t pos = output.find("Content-Type: text/html");
            if (pos != std::string::npos)
            {
                output.erase(pos, std::string("Content-Type: text/html").length());
            }
            cgiOutput.str(output);
            res.setBody(cgiOutput.str());
            res.setMimeType("html");
            res.setStatusCode(200);
        }
        // std::cout << "CGI OUTPUT: " << cgiOutput.str() << std::endl;
    }
    freeEnv(env);
}
