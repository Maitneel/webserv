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

char **create_argv(const std::string &program_name, const std::string &cgi_script_path) {
    size_t allocation_size = 2;
    if (program_name != cgi_script_path) {
        allocation_size++;
    }
    char **argv = reinterpret_cast<char **>(malloc(sizeof(char *) * allocation_size));
    if (argv == NULL) {
        exit(127);
    }
    size_t i = 0;
    if (program_name != cgi_script_path) {
        argv[i] = strdup(program_name.c_str());
        if (argv[i++] == NULL) {
            exit(127);
        }
    }
    argv[i] = strdup(cgi_script_path.c_str());
    if (argv[i++] == NULL) {
        exit(127);
    }
    argv[i++] = NULL;
    return argv;
}

void child_process(const HTTPRequest &request, const std::string &cgi_script_path, const std::string &path_info, const int &unix_socet_fd) {
    dup2(unix_socet_fd, STDIN_FILENO);
    dup2(unix_socet_fd, STDOUT_FILENO);
    std::string extension;
    std::string program_name = cgi_script_path;
    const std::string::size_type last_period_index = cgi_script_path.rfind('.');
    if (last_period_index != std::string::npos) {
        extension = cgi_script_path.substr(last_period_index);
    }
#ifdef PHP_PATH
    if (extension == ".php") {
        program_name = PHP_PATH;
    }
#endif
#ifdef PYTHON3_PATH
    if (extension == ".py") {
        program_name = PYTHON3_PATH;
    }
#endif

    char **argv = create_argv(program_name, cgi_script_path);
    char **env = make_env_array(request, path_info);
    execve(program_name.c_str(), argv, env);
    exit(127);
}

CGIInfo call_cgi_script(const HTTPRequest &request, const std::string &cgi_script_path, const std::string &path_info) {
    int sv[2];
    int socketret = socketpair(AF_UNIX, SOCK_STREAM, 0, sv);
    std::cerr << "socketret: " <<  socketret << std::endl;
    if (socketret != 0) {
        throw std::runtime_error("pipe failed");
    }

    const pid_t pid = fork();
    if (pid == 0) {
        close(sv[UNIX_SOCKET_SERVER]);
        child_process(request, cgi_script_path, path_info, sv[UNIX_SOCKET_SCRIPT]);
    }
    const pid_t &child_pid = pid;
    const int from_script = sv[0];
    fcntl(from_script, F_SETFL, O_NONBLOCK | FD_CLOEXEC);
    close(sv[UNIX_SOCKET_SCRIPT]);
    return CGIInfo(sv[UNIX_SOCKET_SERVER], child_pid);
}
