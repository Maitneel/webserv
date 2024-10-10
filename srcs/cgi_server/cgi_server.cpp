#include <unistd.h>

#include <string>
#include <stdexcept>
#include <cstdlib>
#include <cstring>

#include "http_request.hpp"

#define PIPE_READ_FD 0
#define PIPE_WRITE_FD 1

#define BUFFER_SIZE 1024

void close_pipe_fds(const int fds[2]) {
    close(fds[0]);
    close(fds[1]);
}

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

void set_meta_valiable(const HTTPRequest &request) {
    // TODO(maitneel): meta-variable の設定 //
}

void child_process(const HTTPRequest &request, const std::string &cgi_script_path, const int to_script_pipe_fd[2], const int to_server_pipe_fd[2]) {
    set_meta_valiable(request);
    dup2(to_script_pipe_fd[PIPE_READ_FD], STDIN_FILENO);
    dup2(to_server_pipe_fd[PIPE_WRITE_FD], STDOUT_FILENO);
    close_pipe_fds(to_script_pipe_fd);
    close_pipe_fds(to_server_pipe_fd);
    char **argv = create_argv(cgi_script_path);
    execve(cgi_script_path.c_str(), argv, NULL);
    exit(127);
}


void write_body_to_script(const HTTPRequest &request, const int &fd) {
    // TODO(maitneel): bodyの書き込み //
}

std::string read_cgi_responce(const int &fd) {
    // TODO(maitneel): CGIの結果の読み込み //
    // RFC3875 では何種類かresponceが定義されているが、とりま、document responceしか考慮しない //
    std::string document_responce;
    char buffer[BUFFER_SIZE];
    bzero(buffer, BUFFER_SIZE);
    // ここのread, 0帰ってきたら終了でほんとにいいのかわからない //
    while (0 < read(fd, buffer, BUFFER_SIZE)) {
        document_responce += buffer;
        bzero(buffer, BUFFER_SIZE);
    }
    return document_responce;
}

std::string call_cgi_script(const HTTPRequest &request, const std::string &cgi_script_path) {
    int to_script_pipe_fd[2];
    int to_server_pipe_fd[2];
    if (pipe(to_script_pipe_fd) || pipe(to_server_pipe_fd)) {
        throw std::runtime_error("pipe failed");
    }
    const pid_t child_pid = fork();
    if (child_pid == 0) {
        child_process(request, cgi_script_path, to_script_pipe_fd, to_server_pipe_fd);
    }
    const int to_script = to_script_pipe_fd[PIPE_WRITE_FD];
    const int from_script = to_server_pipe_fd[PIPE_READ_FD];
    write_body_to_script(request, to_script);
    close_pipe_fds(to_script_pipe_fd);
    close(to_server_pipe_fd[PIPE_WRITE_FD]);

    waitpid(child_pid, NULL, 0);  // TODO(maitneel): ブロッキングしないようにする //
    std::string cgi_responce = read_cgi_responce(from_script);
    close(from_script);

    return cgi_responce;
}
