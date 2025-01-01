#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/wait.h>
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
using std::cerr;
using std::endl;

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
        if (fcntl(sock, F_SETFL, O_NONBLOCK | FD_CLOEXEC)) {
            std::runtime_error("fcntl: failed");
        }
        return sock;
    }
    return -1;
}

Server::Server(std::map<ServerConfigKey, ServerConfig> confs) {
    std::map<ServerConfigKey, ServerConfig>::iterator it;
    for (it = confs.begin(); it != confs.end(); it++) {
        int sock = create_inet_socket(it->second.port_);
        if (sock < 0)
            throw std::runtime_error("can not create tcp socket.");
        sockets_.push_back(Socket(sock, it->second));  // 謎 //
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

// TODO(maitneel): エラーの場合、exception投げた方が適切かもせ入れない　 //
int Server::GetHandler(int sock, const HTTPRequest& req) {
    ServerConfig conf = this->GetConfigByFd(sock);
    std::string path = conf.document_root_ + req.get_request_uri();

    if (IsDir(path.c_str())) {
        // TODO(taksaito): autoindex か、 index をみるようにする
        // 現在は一旦、index.html をみるように処理
        path += "/index.html";
    }

    std::cout << path << std::endl;
    if (access(path.c_str(), F_OK) == -1) {
        return -2;
    }

    int fd = open(path.c_str(), (O_RDONLY | O_NONBLOCK | O_CLOEXEC));
    if (0 <= fd) {
        return fd;
    }
    return -1;
}

int ft_accept(int fd) {
    struct sockaddr_storage addr;
    socklen_t addrlen = sizeof addr;
    int sock = accept(fd, (struct sockaddr*)&addr, &addrlen);
    if (sock < 0) {
        throw std::runtime_error("accept: failed");
    }
    if (fcntl(sock, F_SETFL, O_NONBLOCK | FD_CLOEXEC)) {
        std::runtime_error("fcntl: failed");
    }
    return sock;
}


void Server::CallCGI(const int &connection_fd, const HTTPRequest &req, const std::string &cgi_path) {
    std::cerr << "call cgi" << std::endl;
    HTTPContext &context = ctxs_.at(connection_fd);
    context.cgi_info_ = call_cgi_script(req, cgi_path);
    context.is_cgi_ = true;
    const CGIInfo &cgi_info = context.cgi_info_;
    dispatcher_.RegisterFileFd(cgi_info.fd, connection_fd);
    dispatcher_.add_writen_buffer(cgi_info.fd, req.entity_body_);
    debug(cgi_info.fd);
    std::cerr << "cgi end" << std::endl;
}


void Server::routing(const int &connection_fd, const int &socket_fd) {
    HTTPContext& ctx = ctxs_.at(connection_fd);
    const HTTPRequest &req = ctx.GetHTTPRequest();
    req.print_info();

    std::string method = req.get_method();

    HTTPResponse res;
    if (req.get_request_uri() == "/cgi/date.cgi" && method == "GET") {
        CallCGI(connection_fd, req, "./cgi_script/date/date.cgi");
        return;
    } else if (req.get_request_uri().substr(0, req.get_request_uri().find("?")) == "/cgi/echo.cgi") {
        CallCGI(connection_fd, req, "./cgi_script/echo/echo.cgi");
        return;
    } else if (req.get_request_uri().find("/cgi/message_board") != std::string::npos) {
        CallCGI(connection_fd, req, "./cgi_script/message_board/message_board.cgi");
        return;
    } else if (method == "GET") {
        int fd = this->GetHandler(socket_fd, req);
        if (fd == -1) {
            res = HTTPResponse(HTTPResponse::kForbidden, "text/html", "Forbidden");
        } else if (fd == -2) {
            res = HTTPResponse(HTTPResponse::kForbidden, "text/html", "Forbidden");
        } else if (0 <= fd) {
            ctx.file_fd_ = fd;
            dispatcher_.RegisterFileFd(fd, connection_fd);
            return;
        }
    } else {
        res = HTTPResponse(HTTPResponse::kNotImplemented, "text/html", "Not Implemented");
    }
    dispatcher_.add_writen_buffer(connection_fd, res.toString());
}

void Server::InsertEventOfWhenChildProcessEnded(std::multimap<int, ConnectionEvent> *events) {
    for (std::map<int, HTTPContext>::iterator it = ctxs_.begin(); it != ctxs_.end(); it++) {
        HTTPContext &current = it->second;
        CGIInfo &cgi_info = current.cgi_info_;
        cgi_info.is_proccess_end = true;

        int temp_child_exit_code;
        if (current.is_cgi_ && 0 < waitpid(current.cgi_info_.pid, &temp_child_exit_code, WNOHANG)) {
            if (0 <= cgi_info.fd && events->find(cgi_info.fd) == events->end()) {
                // どっちのイベントがいいか正直微妙 //
                events->insert(std::make_pair(cgi_info.fd, ConnectionEvent(kFileEndOfRead, -1, current.GetConnectionFD(), cgi_info.fd)));
                // events->insert(std::make_pair(cgi_info.fd, ConnectionEvent(kServerEventFail, -1, current.GetConnectionFD(),cgi_info.fd)));
            }
        }
    }
}

void Server::CloseConnection(const int connection_fd) {
    const HTTPContext &context = ctxs_.at(connection_fd);
    if (context.file_fd_ != NON_EXIST_FD) {
        dispatcher_.UnregisterFileFd(context.file_fd_);
        close(context.file_fd_);
    }
    if (context.is_cgi_) {
        dispatcher_.UnregisterFileFd(context.cgi_info_.fd);
        close(context.cgi_info_.fd);
    }
    dispatcher_.UnregisterConnectionFd(connection_fd);
    close(connection_fd);

    ctxs_.erase(connection_fd);
}

void Server::SendresponseFromCGIresponse(const int &connection_fd, const std::string &cgi_response_string) {
    if (ctxs_.at(connection_fd).sent_response_) {
        return;
    }
    ctxs_.at(connection_fd).sent_response_ = true;
    CGIResponse cgi_res(cgi_response_string);
    HTTPResponse res = cgi_res.make_http_response();

    dispatcher_.add_writen_buffer(connection_fd, res.toString());
}

void Server::SendresponseFromFile(const int &connection_fd, const std::string &file_content) {
    // TODO(maitneel): content-typeをどうにかする //
    if (ctxs_.at(connection_fd).sent_response_) {
        return;
    }
    ctxs_.at(connection_fd).sent_response_ = true;

    HTTPResponse res(HTTPResponse::kOK, "/", file_content);
    dispatcher_.add_writen_buffer(connection_fd, res.toString());
}

void Server::EventLoop() {
    for (size_t i = 0; i < sockets_.size(); i++) {
        dispatcher_.RegisterSocketFd(sockets_[i].GetSocketFd());
    }

    while(true) {
        std::multimap<int, ConnectionEvent> dis_events = dispatcher_.Wait(-1);
        std::multimap<int, ConnectionEvent>::const_iterator it;

        for (it = dis_events.find(PROCESS_CHENGED_FD); it->first == PROCESS_CHENGED_FD; it++) {
            if (it->second.event == kChildProcessChanged) {
                this->InsertEventOfWhenChildProcessEnded(&dis_events);
                break;
            }
        }
        for (it = dis_events.begin(); it != dis_events.end(); it++) {
            const int &event_fd = it->first;
            const ConnectionEvent &event = it->second;

            try {
                cerr << "[fd, event]: " << event_fd << ", " << event.event << ", " << ctxs_.at(event.connection_fd).GetHTTPRequest().get_request_uri() << endl;
            } catch (...) {
                cerr << endl;
            }

            if (event_fd == PROCESS_CHENGED_FD) {
                continue;
            }
            if (ctxs_.find(event.connection_fd) == ctxs_.end()) {
                ctxs_.insert(std::make_pair(event.connection_fd, HTTPContext(event.connection_fd)));
            }
            if (event.event == kUnknownEvent) {
                // Nothing to do;
            } else if (event.event == kReadableRequest || event.event == kRequestEndOfReadad) {  // TODO(maitneel): この中の処理を関数に分けて、ifの条件を一つだけにする
                HTTPContext& ctx = ctxs_.at(event_fd);

                // TODO(maitneel): 辻褄合わせをどうにかする //
                ctx.AppendBuffer(dispatcher_.get_read_buffer(event_fd));
                dispatcher_.erase_read_buffer(event_fd, 0, std::string::npos);

                if (ctx.IsParsedHeader() == false) {
                    if (ctx.GetBuffer().find("\r\n\r\n") != std::string::npos) {
                        ctx.ParseRequestHeader();
                    }
                }
                if (ctx.IsParsedHeader() && ctx.body_.IsComplated()) {
                    ctx.ParseRequestBody();
                    this->routing(event_fd, it->second.socket_fd);
                }
            } else if (event.event == kReadableFile) {
                // TODO(maitneel): Do it;
            } else if (event.event == kFileEndOfRead) {
                // TODO(maitneel): Do it;
                if (ctxs_.at(event.connection_fd).is_cgi_ && ctxs_.at(event.connection_fd).cgi_info_.is_proccess_end) {
                    this->SendresponseFromCGIresponse(event.connection_fd, dispatcher_.get_read_buffer(event_fd));
                    dispatcher_.UnregisterFileFd(event_fd);
                } else if (!ctxs_.at(event.connection_fd).is_cgi_) {
                    this->SendresponseFromFile(event.connection_fd, dispatcher_.get_read_buffer(event_fd));
                    dispatcher_.UnregisterFileFd(event_fd);
                }
            } else if (event.event == kFileEndOfRead) {
                if (ctxs_.at(event.connection_fd).is_cgi_ && ctxs_.at(event.connection_fd).cgi_info_.is_proccess_end) {
                    this->SendresponseFromCGIresponse(event.connection_fd, dispatcher_.get_read_buffer(event_fd));
                    dispatcher_.UnregisterFileFd(event_fd);
                } else if (!ctxs_.at(event.connection_fd).is_cgi_) {
                    this->SendresponseFromFile(event.connection_fd, dispatcher_.get_read_buffer(event_fd));
                    dispatcher_.UnregisterFileFd(event_fd);
                }
            } else if (event.event == kresponseWriteEnd) {
                // TODO(maitneel): Do it (これだけでいいのか？？？);
                // fdのclose忘れが出てきたらここが原因 //
                // cerr << "resend: " << event_fd << endl;
                this->CloseConnection(event.connection_fd);
            } else if (event.event == kFileWriteEnd) {
                // TODO(maitneel): Do it;
            } else if (event.event == kChildProcessChanged) {
                // Nothing to do (processed)
            } else if (event.event == kServerEventFail) {
                // TODO(maitneel): Do it;
                if (event_fd == event.connection_fd) {
                    this->CloseConnection(event.connection_fd);
                } else if (event_fd == event.file_fd) {
                    if (ctxs_.find(event.connection_fd) != ctxs_.end() && ctxs_.at(event.connection_fd).is_cgi_) {
                        this->SendresponseFromCGIresponse(event.connection_fd, dispatcher_.get_read_buffer(event_fd));
                    }
                    dispatcher_.UnregisterFileFd(event_fd);
                }
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
