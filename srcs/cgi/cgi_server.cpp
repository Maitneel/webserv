#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <string>
#include <stdexcept>
#include <cstdlib>
#include <cstring>
#include <iostream>

#include "http_request.hpp"
#include <cgi.hpp>

#define PIPE_READ_FD 0
#define PIPE_WRITE_FD 1

#define UNIX_SOCKET_SERVER 0
#define UNIX_SOCKET_SCRIPT 1

#define BUFFER_SIZE 1024

char **create_argv(const std::string &cgi_script_path) {
    char **argv = reinterpret_cast<char **>(malloc(sizeof(char *) * 2));
    if (argv == NULL) {
        exit(127);
    }
    argv[0] = strdup(cgi_script_path.c_str());
    argv[1] = NULL;
    if (argv[0] == NULL) {
        exit(127);
    }
    return argv;
}

void child_process(const HTTPRequest &request, const std::string &cgi_script_path, const int &unix_socet_fd) {
    dup2(unix_socet_fd, STDIN_FILENO);
    dup2(unix_socet_fd, STDOUT_FILENO);
    char **argv = create_argv(cgi_script_path);
    char **env = make_env_array(request);
    execve(cgi_script_path.c_str(), argv, env);
    exit(127);
}

CGIInfo call_cgi_script(const HTTPRequest &request, const std::string &cgi_script_path) {
    int sv[2];
    int socketret = socketpair(AF_UNIX, SOCK_STREAM, 0, sv);
    std::cerr << "socketret: " <<  socketret << std::endl;
    if (socketret != 0) {
        throw std::runtime_error("pipe failed");
    }

    const pid_t pid = fork();
    if (pid == 0) {
        close(sv[UNIX_SOCKET_SERVER]);
        child_process(request, cgi_script_path, sv[UNIX_SOCKET_SCRIPT]);
    }
    const pid_t &child_pid = pid;
    const int to_script = sv[1];
    const int from_script = sv[0];
    fcntl(from_script, F_SETFL, O_NONBLOCK | FD_CLOEXEC);
    close(sv[UNIX_SOCKET_SCRIPT]);
    return CGIInfo(sv[UNIX_SOCKET_SERVER], child_pid);
}
