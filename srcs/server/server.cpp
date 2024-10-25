#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netdb.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <cstdlib>
#include <cstring>

#include <string>
#include <sstream>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <stdexcept>
#include <vector>
#include <map>
#include <utility>

#include "server.hpp"
#include "poll_selector.hpp"
#include "http_request.hpp"
#include "http_response.hpp"
#include "cgi_response.hpp"
#include "cgi.hpp"

#define debug(s) std::cerr << #s << '\'' << (s) << '\'' << std::endl;

std::string int_to_str(int n) {
    std::stringstream ss;
    ss << n;
    return ss.str();
}

std::string GetContent(const std::string& path) {
    std::ifstream ifs(path.c_str());
    if (ifs.fail()) {
        ifs.close();
        throw std::invalid_argument("can not open file " + path);
    }

    std::stringstream ss;
    ss << ifs.rdbuf();
    return ss.str();
}

std::string get_formated_date() {
    struct tm newtime;
    time_t ltime;
    char buf[50];

    ltime = time(&ltime);
    localtime_r(&ltime, &newtime);
    std::string now_string(asctime_r(&newtime, buf));
    now_string.erase(
        remove(now_string.begin(), now_string.end(), '\n'),
        now_string.end());
    return now_string;
}

std::string read_request(int fd) {
    char buf[BUFFER_SIZE];
    std::string content;
    int n_read = recv(fd, buf, BUFFER_SIZE-1, 0);
    debug(n_read);
    if (n_read < 0) {
        return "";
    }
    buf[n_read] = '\0';
    return std::string(buf, n_read);
}

void response_to_client(int fd, const HTTPResponse& response) {
    std::string response_raw = response.toString();
    if (send(fd, response_raw.c_str(), response_raw.length(), 0) != 0)
        close(fd);
    return;
}

int create_inet_socket(int port) {
    struct addrinfo hints, *res, *ai;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if (getaddrinfo(NULL, int_to_str(port).c_str(), &hints, &res) != 0) {
        return -1;
    }

    for (ai = res; ai; ai=ai->ai_next) {
        int sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (sock < 0) {
            close(sock);
            continue;
        }
        // 本番環境として動かすならやらない方がいいと思うので、プリプロセッサで条件分岐した方がいいかもです //
        int sockopt_arg = 1;
        if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &sockopt_arg, sizeof(int))) {
            std::cerr << "setsockopt: failed set SO_REUSEADDR option" << std::endl;
        }
        if (bind(sock, res->ai_addr, res->ai_addrlen) < 0) {
            close(sock);
            continue;
        }
        if (listen(sock, MAX_BACKLOG) < 0) {
            close(sock);
            continue;
        }
        freeaddrinfo(res);
        if (fcntl(sock, F_SETFL, O_NONBLOCK)) {
            std::runtime_error("fcntl: failed");
        }
        return sock;
    }
    return -1;
}

Server::Server(std::vector<ServerConfig> confs) {
    std::vector<ServerConfig>::iterator it;
    for (it = confs.begin(); it != confs.end(); it++) {
        int sock = create_inet_socket(it->port);
        if (sock < 0)
            throw std::runtime_error("can not create tcp socket.");
        sockets_.push_back(Socket(sock, *it));
    }
}

Server::~Server() {}

Socket::Socket(int socket_fd, ServerConfig config): socket_fd(socket_fd), config(config) {}

Socket::~Socket() {}

int Socket::GetSocketFd() {
    return this->socket_fd;
}

const ServerConfig& Socket::GetConfig() {
    return this->config;
}


ServerConfig Server::GetConfigByFd(int fd) {
    std::vector<Socket>::iterator it;
    for (it = sockets_.begin(); it != sockets_.end(); it++) {
        if (it->GetSocketFd() == fd)
            return it->GetConfig();
    }
    throw std::invalid_argument("invalid fd");
}

std::string GetContentType(const std::string path) {
    std::string::size_type dot_pos = path.rfind(".");
    if (dot_pos == std::string::npos) {
        return "application/octet-stream";
    }
    std::string ext = path.substr(dot_pos + 1);

    if (ext == "html") {
        return "text/html";
    } else if (ext == "txt") {
        return "text/plain";
} else if (ext == "png") {
        return "image/png";
    }
    // TODO(taksaito): 他の MIME タイプの対応
    return "application/octet-stream";
}

bool IsDir(const std::string& path) {
    struct stat st;
    if (stat(path.c_str(), &st) != 0)
        return false;
    return (st.st_mode & S_IFMT) == S_IFDIR;
}

HTTPResponse Server::GetHandler(int sock, const HTTPRequest& req) {
    ServerConfig conf = this->GetConfigByFd(sock);
    std::string path = conf.document_root + req.get_request_uri();

    if (IsDir(path.c_str())) {
        // TODO(taksaito): autoindex か、 index をみるようにする
        // 現在は一旦、index.html をみるように処理
        path += "/index.html";
    }

    std::cout << path << std::endl;
    if (access(path.c_str(), F_OK) == -1) {
        return HTTPResponse(HTTPResponse::kNotFound, "text/html", "Not Found");
    }

    std::string content;
    try {
        content = GetContent(path);
    } catch (std::invalid_argument& e) {
        return HTTPResponse(HTTPResponse::kForbidden, "text/html", "Forbidden");
    }
    return HTTPResponse(HTTPResponse::kOK, GetContentType(path), content);
}

int ft_accept(int fd) {
    struct sockaddr_storage addr;
    socklen_t addrlen = sizeof addr;
    int sock = accept(fd, (struct sockaddr*)&addr, &addrlen);
    if (sock < 0) {
        throw std::runtime_error("accept: failed");
    }
    if (fcntl(sock, F_SETFL, O_NONBLOCK)) {
        std::runtime_error("fcntl: failed");
    }
    return sock;
}


//  雑of雑なので作り直さないといけないと思う //
HTTPResponse create_cgi_responce(const HTTPRequest &req, const std::string &cgi_path) {
    CGIResponse cgi_res(call_cgi_script(req, cgi_path));
    return cgi_res.make_http_response();
}

void Server::AcceptRequest(int fd) {
    int accepted_fd = ft_accept(fd);
    selector_.Register(accepted_fd, kEventRead);
    ctxs_.insert(std::make_pair(accepted_fd, HTTPContext(fd)));
}

void Server::EventLoop() {
    for (size_t i = 0; i < sockets_.size(); i++) {
        selector_.Register(sockets_[i].GetSocketFd(), kEventRead);
    }

    while(true) {
        std::vector<FDEvent> events;
        events = selector_.Select(100);

        std::vector<FDEvent>::const_iterator it;
        for (it = events.begin(); it != events.end(); it++) {
            if (this->IsIncludeFd(it->fd) && it->event == kEventRead) {
                try {
                    this->AcceptRequest(it->fd);
                } catch (std::exception& e) {
                    std::cerr << e.what() << std::endl;
                    continue;
                }
            } else if (it->event == kEventRead) {
                HTTPContext& ctx = ctxs_.at(it->fd);
                ctx.AppendBuffer(read_request(it->fd));
                std::cerr << ctx.GetBuffer() << std::endl;

                if (ctx.IsParsedHeader() == false) {
                    if (ctx.GetBuffer().find("\r\n\r\n") != std::string::npos) {
                        ctx.ParseRequestHeader();
                    }
                }
                if (ctx.IsParsedHeader() && ctx.GetHTTPRequest().content_length_ <= ctx.GetBuffer().length()) {
                    ctx.ParseRequestBody();
                    selector_.Register(it->fd, kEventWrite);
                }
            } else if (it->event == kEventWrite) {
                HTTPContext& ctx = ctxs_.at(it->fd);
                HTTPRequest &req = ctx.GetHTTPRequest();
                req.print_info();

                std::string method = req.get_method();

                HTTPResponse res;
                if (req.get_request_uri() == "/cgi/date.cgi" && method == "GET") {
                    res = create_cgi_responce(req, "./cgi_script/date/date.cgi");
                } else if (req.get_request_uri().substr(0, req.get_request_uri().find("?")) == "/cgi/echo.cgi") {
                    res = create_cgi_responce(req, "./cgi_script/echo/echo.cgi");
                } else if (req.get_request_uri().find("/cgi/message_board") != std::string::npos) {
                    res = create_cgi_responce(req, "./cgi_script/message_board/message_board.cgi");
                } else if (method == "GET") {
                    res = this->GetHandler(ctx.GetSocketFD(), req);
                } else {
                    res = HTTPResponse(HTTPResponse::kNotImplemented, "text/html", "Not Implemented");
                }
                response_to_client(it->fd, res);
                selector_.Unregister(it->fd);
                close(it->fd);
                // keep-alive のことを考えるならeraseするべきではないかもしれない //
                ctxs_.erase(it->fd);
            } else if (it->event == kEvnetError) {
                ctxs_.erase(it->fd);
                close(it->fd);
            }
        }
    }
}

bool Server::IsIncludeFd(int fd) {
    for (size_t i = 0; i < sockets_.size(); i++) {
        if (sockets_[i].GetSocketFd() == fd)
            return true;
    }
    return false;
}
