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

#define SOCKET_SERVER_WRITE_FD 0
#define SOCKET_SERVER_READ_FD 1

#define SOCKET_SCRIPT_WRITE_FD 1
#define SOCKET_SCRIPT_READ_FD 0

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

void child_process(const HTTPRequest &request, const std::string &cgi_script_path, const int to_script_pipe_fd, const int to_server_pipe_fd) {
    dup2(to_script_pipe_fd, STDIN_FILENO);
    dup2(to_server_pipe_fd, STDOUT_FILENO);
    char **argv = create_argv(cgi_script_path);
    char **env = make_env_array(request);
    execve(cgi_script_path.c_str(), argv, env);
    exit(127);
}


// void write_body_to_script(const HTTPRequest &request, const int &fd) {
//     const std::string body = request.entity_body_;
//     int remining_date = request.entity_body_.length();
//     // TODO(maitneel): bug: 全て書き込めなかった場合に最初から書き込み直している　//
//     // あとノンブロッキングじゃない //
//     do {
//         int write_ret = write(fd, body.c_str(), remining_date);
//         if (write_ret < 0) {
//             throw std::runtime_error("write");
//         }
//         remining_date -= write_ret;
//     } while (remining_date);
// }

// std::string read_cgi_responce(const int &fd) {
//     // TODO(maitneel): CGIの結果の読み込み //
//     // RFC3875 では何種類かresponceが定義されているが、とりま、document responceしか考慮しない //
//     std::string document_responce;
//     char buffer[BUFFER_SIZE];
//     bzero(buffer, BUFFER_SIZE);
//     // ここのread, 0帰ってきたら終了でほんとにいいのかわからない //
//     while (0 < read(fd, buffer, BUFFER_SIZE)) {
//         document_responce += buffer;
//         bzero(buffer, BUFFER_SIZE);
//     }
//     return document_responce;
// }

CGIInfo call_cgi_script(const HTTPRequest &request, const std::string &cgi_script_path) {
    int sv[2];
    int socketret = socketpair(AF_UNIX, SOCK_STREAM, 0, sv);
    std::cerr << "socketret: " <<  socketret << std::endl;
    if (socketret != 0) {
        throw std::runtime_error("pipe failed");
    }

    const pid_t child_pid = fork();
    if (child_pid == 0) {
        child_process(request, cgi_script_path, sv[SOCKET_SCRIPT_READ_FD], sv[SOCKET_SCRIPT_WRITE_FD]);
    }
    const int to_script = sv[SOCKET_SERVER_WRITE_FD];
    const int from_script = sv[SOCKET_SERVER_READ_FD];
    fcntl(from_script, F_SETFL, O_NONBLOCK);

    return CGIInfo(to_script, from_script, child_pid);
}
