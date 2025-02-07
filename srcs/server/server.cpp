#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <signal.h>
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
#include <set>
#include <utility>

#include "server.hpp"
#include "poll_selector.hpp"
#include "http_exception.hpp"
#include "http_request.hpp"
#include "http_response.hpp"
#include "cgi_response.hpp"
#include "cgi.hpp"
#include "file_signatures.hpp"

#define debug(s) std::cerr << #s << '\'' << (s) << '\'' << std::endl;
using std::cerr;
using std::endl;

std::string int_to_str(int n) {
    std::stringstream ss;
    ss << n;
    return ss.str();
}

const FileSignatures file_signatures;

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

// void response_to_client(int fd, const HTTPResponse& response) {
//     std::string response_raw = response.toString();
//     if (send(fd, response_raw.c_str(), response_raw.length(), 0) != 0)
//         close(fd);
//     return;
// }

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

    std::set<int> created_port;
    for (it = confs.begin(); it != confs.end(); it++) {
        ServerConfig &config = it->second;
        int port = config.port_;
        if (created_port.find(port) == created_port.end()) {
            int sock = create_inet_socket(port);
            if (sock < 0) {
                throw std::runtime_error("can not create tcp socket.");
            }
            created_port.insert(port);
            socket_list_.AddSocket(port, sock);
        }
    }
    this->config_ = confs;
}

Server::~Server() {}

SocketList::SocketList() {
}

SocketList::~SocketList() {
}

void SocketList::AddSocket(const int &port, const int &fd) {
    port_fd_pair_.insert(std::make_pair(port, fd));
    fd_port_pair_.insert(std::make_pair(fd, port));
}

int SocketList::GetPort(const int &fd) {
    const std::map<int, int>::const_iterator it = fd_port_pair_.find(fd);

    if (it != fd_port_pair_.end()) {
        return it->second;
    }
    return NON_EXIST_FD;
}

int SocketList::GetFd(const int &port) {
    const std::map<int, int>::const_iterator it = port_fd_pair_.find(port);

    if (it != port_fd_pair_.end()) {
        return it->second;
    }
    return NON_EXIST_FD;
}

std::string get_extension(const std::string file_name) {
    const std::string::size_type last_slash_index = file_name.rfind('/');
    const std::string::size_type last_dot_index = file_name.rfind('.');

    if (last_slash_index == std::string::npos) {
        if (last_dot_index != std::string::npos) {
            return file_name.substr(last_dot_index);
        } else {
            return "";
        }
    } else {
        if (last_slash_index < last_dot_index) {
            return file_name.substr(last_dot_index);
        } else {
            return "";
        }
    }
}

std::string GetContentType(const std::string path) {
    std::string content_type = "";
    try {
        content_type = file_signatures.GetMIMEType(path);
    } catch (std::runtime_error &e) {
        // nothing to do;
    }
    if (content_type != "" && content_type != "application/octet-stream") {
        return content_type;
    }

    std::string ext = get_extension(path);

    if (ext == "html") {
        return "text/html";
    } else if (ext == "txt") {
        return "text/plain";
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
void Server::GetMethodHandler(HTTPContext *context, const std::string &req_path, const ServerConfig &server_config, const LocatoinConfig &location_config) {
    const std::string &document_root = location_config.document_root_;
    std::string path = document_root + req_path;
    const int &connection_fd = context->GetConnectionFD();

    if (IsDir(path.c_str())) {
        // TODO(taksaito): autoindex か、 index をみるようにする
        // 現在は一旦、index.html をみるように処理
        if (location_config.autoindex_) {
            try {
                HTTPResponse res(HTTPResponse::kOK, "text/html", generate_autoindex_file(path, req_path));
                dispatcher_.add_writen_buffer(connection_fd, res.toString());
            } catch (MustReturnHTTPStatus &e) {
                this->SendErrorResponce(convert_status_code_to_enum(e.GetStatusCode()), server_config, connection_fd);
            }
            return;
        }

        path += *(location_config.index_.begin());
    }
    if (path.at(path.length() - 1) == '/') {
        path.erase(path.length() - 1);
    }
    // std::cout << "path: " << path << std::endl;
    if (access(path.c_str(), F_OK) == -1) {
        this->SendErrorResponce(HTTPResponse::kNotFound, server_config, connection_fd);
        return;
    }
    if (access(path.c_str(), R_OK) == -1) {
        this->SendErrorResponce(HTTPResponse::kForbidden, server_config, connection_fd);
        return;
    }

    int fd = open(path.c_str(), (O_RDONLY | O_NONBLOCK | O_CLOEXEC));
    if (0 <= fd) {
        context->content_type = GetContentType(path);
        context->file_fd_ = fd;
        dispatcher_.RegisterFileFd(fd, connection_fd);
        return;
    }
    this->SendErrorResponce(HTTPResponse::kBadRequest, server_config, connection_fd);
    return;
}

void Server::HeadMethodHandler(HTTPContext *context, const std::string &req_path, const ServerConfig &server_config, const LocatoinConfig &location_config) {
    const std::string &document_root = location_config.document_root_;
    std::string path = document_root + req_path;
    const int &connection_fd = context->GetConnectionFD();

    if (IsDir(path.c_str())) {
        if (location_config.autoindex_) {
            if (access(path.c_str(), R_OK) == -1) {
                this->SendErrorResponce(403, server_config, connection_fd);
            } else {
                HTTPResponse res(HTTPResponse::kOK, "text/html", "");
                this->dispatcher_.add_writen_buffer(connection_fd, res.toString());
            }
            return;
        }
        path += "/index.html";
    }

    std::cout << path << std::endl;
    if (access(path.c_str(), F_OK) == -1) {
        this->SendErrorResponce(HTTPResponse::kBadRequest, server_config, connection_fd);
        return;
    }
    if (access(path.c_str(), R_OK) == -1) {
        this->SendErrorResponce(HTTPResponse::kForbidden, server_config, connection_fd);
        return;
    }

    HTTPResponse res(HTTPResponse::kOK, GetContentType(path), "");
    this->dispatcher_.add_writen_buffer(connection_fd, res.toString());
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
    // cerr << "accept: [sock, con]: [" << fd << ", " << sock << "]" << endl;
    return sock;
}


void Server::CallCGI(const int &connection_fd, HTTPRequest *req, const std::string &cgi_path, const std::string &loc_name) {
    std::cerr << "call cgi" << std::endl;
    std::string path_info = "/";
    std::string req_path = req->get_request_uri();
    if (req_path.at(req_path.length() - 1) != '/') {
        req_path += '/';
    }
    if (loc_name.length() < req_path.length()) {
        path_info = req_path.substr(loc_name.length());
    }
    HTTPContext &context = ctxs_.at(connection_fd);
    context.cgi_info_ = call_cgi_script(*req, cgi_path, path_info);
    context.is_cgi_ = true;
    const CGIInfo &cgi_info = context.cgi_info_;
    dispatcher_.RegisterFileFd(cgi_info.fd, connection_fd);
    if (req->entity_body_.length() != 0) {
        dispatcher_.add_writen_buffer(cgi_info.fd, req->entity_body_);
        req->entity_body_.clear();
    } else {
        shutdown(cgi_info.fd, SHUT_WR);
    }
    debug(cgi_info.fd);
    std::cerr << "cgi end" << std::endl;
}

const LocatoinConfig &Server::GetLocationConfig(const int &port, const HTTPRequest &req) {
    std::string location = req.get_request_uri().substr(0, req.get_request_uri().find('?'));
    if (location.length() == 0 || location.at(location.length() - 1) != '/') {
        location += "/";
    }
    const ServerConfig &server_config = GetConfig(port, req.get_host_name());

    std::string::size_type location_length = location.rfind('/');
    std::map<std::string, LocatoinConfig>::const_iterator location_config_it = server_config.location_configs_.end();
    while (location_length != std::string::npos) {
        location_config_it = server_config.location_configs_.find(location.substr(0, location_length + 1));
        if (location_config_it != server_config.location_configs_.end()) {
            break;
        }
        if (location_length == 0) {
            break;
        }
        location_length = location.rfind('/', location_length - 1);
    }
    return location_config_it->second;
}

void Server::RoutingByLocationConfig(HTTPContext *ctx, const ServerConfig &server_config, const LocatoinConfig &loc_conf, const std::string &req_uri, const int &connection_fd) {
    // const HTTPRequest *req = ctx->GetHTTPRequest();
    HTTPRequest *req = &ctx->request_;
    const std::string method = req->get_method();
    if (loc_conf.methods_.find(method) == loc_conf.methods_.end()) {
        this->SendErrorResponce(HTTPResponse::kMethodNotAllowed, server_config, connection_fd);
        return;
    }
    if (loc_conf.cgi_path_ != "") {
        this->CallCGI(connection_fd, req, loc_conf.cgi_path_, loc_conf.name_);
        return;
    } else if (method == "GET") {
        this->GetMethodHandler(ctx, req_uri, server_config, loc_conf);
        return;
    } else if (method == "HEAD") {
        this->HeadMethodHandler(ctx, req_uri, server_config, loc_conf);
    } else {
        SendErrorResponce(HTTPResponse::kBadRequest, server_config, connection_fd);
    }
}

void Server::routing(const int &connection_fd, const int &socket_fd) {
    HTTPContext& ctx = ctxs_.at(connection_fd);
    const HTTPRequest &req = ctx.GetHTTPRequest();
    const int port = socket_list_.GetPort(socket_fd);
    // req.print_info();
    std::string host_name = req.get_host_name();
    // TODO(maitneel): origin-form以外に対応できていない //
    const std::string &req_uri = req.get_request_uri().substr(0, req.get_request_uri().find('?'));
    std::string location = req_uri;
    if (location.length() == 0 || location.at(location.length() - 1) != '/') {
        location += "/";
    }

    ServerConfig config;
    try {
        config = this->GetConfig(port, host_name);
    } catch (std::runtime_error &e) {
        this->SendErrorResponce(HTTPResponse::kNotFound, config, connection_fd);
        return;
    }

    if (req_uri.find("..") != std::string::npos) {
        this->SendErrorResponce(HTTPResponse::kBadRequest, config, connection_fd);
        return;
    }

    std::string method = req.get_method();

    try {
        LocatoinConfig location_config = GetLocationConfig(port, req);
        std::string::size_type location_length = location_config.name_.length();
        this->RoutingByLocationConfig(&ctx, config, location_config, location.substr(location_length), connection_fd);
    } catch (...) {
        SendErrorResponce(404, config, connection_fd);
    }
}

void Server::HandlingChildPID() {
    int temp_child_exit_code;
    std::set<pid_t>::const_iterator it = pid_killed_by_webserve_.begin();
    while (it != pid_killed_by_webserve_.end()) {
        pid_t pid = *it;
        it++;
        if (waitpid(pid, &temp_child_exit_code, WNOHANG)) {
            pid_killed_by_webserve_.erase(pid);
        }
    }

    for (std::map<int, HTTPContext>::iterator it = ctxs_.begin(); it != ctxs_.end(); it++) {
        HTTPContext &current = it->second;
        CGIInfo &cgi_info = current.cgi_info_;
        cgi_info.is_proccess_end = true;

        if (current.is_cgi_ && 0 < waitpid(current.cgi_info_.pid, &temp_child_exit_code, WNOHANG)) {
            pid_killed_by_webserve_.erase(current.cgi_info_.pid);
        }
    }
}

int fail_close = 0;
int resend_close = 0;

void Server::CloseConnection(const int connection_fd) {
    // cerr << "[success, fail]: [" << resend_close << ", " << fail_close << "]" << endl;
    const HTTPContext &context = ctxs_.at(connection_fd);
    if (context.file_fd_ != NON_EXIST_FD) {
        dispatcher_.UnregisterFileFd(context.file_fd_);
        close(context.file_fd_);
    }
    if (context.cgi_info_.fd != NON_EXIST_FD) {
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

void Server::SendresponseFromFile(const int &connection_fd, const std::string &file_content, const std::string &content_type) {
    // TODO(maitneel): content-typeをどうにかする //
    if (ctxs_.at(connection_fd).sent_response_) {
        return;
    }
    ctxs_.at(connection_fd).sent_response_ = true;

    HTTPResponse res(HTTPResponse::kOK, content_type, file_content);
    dispatcher_.add_writen_buffer(connection_fd, res.toString());
}

void Server::SendErrorResponce(const int &stat, const ServerConfig config, const int &connection_fd) {
    // TODO(maitneel): エラーページを返すようにする //
    std::string error_message;

    if (stat == HTTPResponse::kBadRequest) {
        error_message = "BadRequest";
    }
    if (stat == HTTPResponse::kForbidden) {
        error_message = "Forbidden";
    }
    if (stat == HTTPResponse::kNotFound) {
        error_message = "NotFound";
    }
    if (stat == HTTPResponse::kMethodNotAllowed) {
        error_message = "MethodNotAllowed";
    }
    if (stat == HTTPResponse::kPayloadTooLarge) {
        error_message = "PayloadTooLarge";
    }
    if (stat == HTTPResponse::kRequestTimeout) {
        error_message = "RequestTimeout";
    }
    if (stat == HTTPResponse::kInternalServerErrror) {
        error_message = "InternalServerErrror";
    }
    if (stat == HTTPResponse::kNotImplemented) {
        error_message = "NotImplemented";
    }

    HTTPResponse res(stat, "text/html", error_message);
    dispatcher_.add_writen_buffer(connection_fd, res.toString());

    (void)(config);
}

const ServerConfig &Server::GetConfig(const int &port, const std::string &host_name) {
    ServerConfigKey config_key(port, host_name);
    std::map<ServerConfigKey, ServerConfig>::iterator config_it = config_.find(config_key);
    if (config_it != config_.end()) {
        return config_it->second;
    } else {
        // TODO(maitneel): 例外をいいかんじのやつにする //
        throw std::runtime_error("no exist config");
    }
}

void Server::EventLoop() {
    for (std::map<ServerConfigKey, ServerConfig>::const_iterator it = config_.begin(); it != config_.end(); it++) {
        dispatcher_.RegisterSocketFd(socket_list_.GetFd(it->second.port_));
    }

    while(true) {
        std::multimap<int, ConnectionEvent> dis_events = dispatcher_.Wait(-1);
        std::multimap<int, ConnectionEvent>::const_iterator it;

        for (it = dis_events.find(PROCESS_CHENGED_FD); it->first == PROCESS_CHENGED_FD; it++) {
            if (it->second.event == kChildProcessChanged) {
                this->HandlingChildPID();
                break;
            }
        }
        for (it = dis_events.begin(); it != dis_events.end(); it++) {
            const int &event_fd = it->first;
            const ConnectionEvent &event = it->second;

            // try {
            //     cerr << "[fd, event]: " << event_fd << ", " << event.event << ", " << ctxs_.at(event.connection_fd).GetHTTPRequest().get_request_uri() << endl;
            // } catch (...) {
            //     cerr << endl;
            // }

            if (event_fd == PROCESS_CHENGED_FD) {
                continue;
            }
            if (ctxs_.find(event.connection_fd) == ctxs_.end()) {
                ctxs_.insert(std::make_pair(event.connection_fd, HTTPContext(event.connection_fd)));
            }
            if (event.event == kUnknownEvent) {
                // Nothing to do;
            } else if (event.event == kRequestEndOfReaded) {
                if (dispatcher_.IsEmptyWritebleBuffer(event_fd)) {
                    fail_close++;
                    CloseConnection(event_fd);
                } else {
                    dispatcher_.UnregisterConnectionReadEvent(event_fd);
                }
            } else if (event.event == kReadableRequest) {
                HTTPContext& ctx = ctxs_.at(event_fd);
                // if (event.event == kRequestEndOfReaded && ctx.IsParsedBody()) {
                //     CloseConnection(event.connection_fd);
                //     continue;
                // }
                // TODO(maitneel): 辻褄合わせをどうにかする //
                try {
                    ctx.AppendBuffer(dispatcher_.get_read_buffer(event_fd));
                } catch (MustReturnHTTPStatus &e) {
                    dispatcher_.UnregisterConnectionReadEvent(event_fd);
                    const ServerConfig server_config = (config_.lower_bound(ServerConfigKey(socket_list_.GetPort(event.socket_fd), "")))->second;
                    this->SendErrorResponce(e.GetStatusCode(), server_config, event.connection_fd);
                    continue;
                }
                dispatcher_.erase_read_buffer(event_fd, 0, std::string::npos);

                if (ctx.IsParsedHeader() == false) {
                    if (ctx.GetBuffer().find("\r\n\r\n") != std::string::npos) {
                        try {
                            const int port = socket_list_.GetPort(event.socket_fd);
                            ctx.ParseRequestHeader(port);
                            ctx.SetMaxBodySize(GetLocationConfig(port, ctx.GetHTTPRequest()).max_body_size_);
                        } catch (const MustReturnHTTPStatus &e) {
                            // TODO(maitneel): default のエラーを返すよにする //
                            dispatcher_.UnregisterConnectionReadEvent(event.connection_fd);
                            const ServerConfig server_config = (config_.lower_bound(ServerConfigKey(socket_list_.GetPort(event.socket_fd), "")))->second;
                            this->SendErrorResponce(e.GetStatusCode(), server_config, event.connection_fd);
                            continue;
                        } catch (std::exception &e) {
                            // TODO(maitneel): ほんとは InvalidHeader　と InvalidRequestだけでいい
                            dispatcher_.UnregisterConnectionReadEvent(event.connection_fd);
                            const ServerConfig server_config = (config_.lower_bound(ServerConfigKey(socket_list_.GetPort(event.socket_fd), "")))->second;
                            this->SendErrorResponce(400, server_config, event.connection_fd);
                            continue;
                        }
                    } else if (event.event == kRequestEndOfReaded) {
                        // CloseConnection(event.connection_fd);
                    }
                }
                if (ctx.IsParsedHeader() && ctx.body_.IsComplated()) {
                    ctx.ParseRequestBody();
                    this->routing(event_fd, it->second.socket_fd);
                } else if (event.event == kRequestEndOfReaded) {
                    // CloseConnection(event.connection_fd);
                }
            } else if (event.event == kReadableFile) {
                // TODO(maitneel): Do it;
            } else if (event.event == kFileEndOfRead) {
                const HTTPContext &ctx = ctxs_.at(event.connection_fd);
                if (ctx.is_cgi_ && ctx.cgi_info_.is_proccess_end) {
                    this->SendresponseFromCGIresponse(event.connection_fd, dispatcher_.get_read_buffer(event_fd));
                    dispatcher_.UnregisterFileFd(event_fd);
                } else if (!ctx.is_cgi_) {
                    this->SendresponseFromFile(event.connection_fd, dispatcher_.get_read_buffer(event_fd), ctx.content_type);
                    dispatcher_.UnregisterFileFd(event_fd);
                }
            } else if (event.event == kresponseWriteEnd) {
                // TODO(maitneel): Do it (これだけでいいのか？？？);
                // fdのclose忘れが出てきたらここが原因 //
                // cerr << "resend: " << event_fd << endl;
                resend_close++;
                this->CloseConnection(event.connection_fd);
            } else if (event.event == kFileWriteEnd) {
                // TODO(maitneel): Do it;
                shutdown(event_fd, SHUT_WR);
            } else if (event.event == kChildProcessChanged) {
                // Nothing to do (processed)
            } else if (event.event == kTimeout) {
                if (dispatcher_.IsEmptyWritebleBuffer(event_fd)) {
                    const HTTPContext &context = ctxs_.at(event_fd);
                    if (context.is_cgi_) {
                        kill(context.cgi_info_.pid, SIGTERM);
                        pid_killed_by_webserve_.insert(context.cgi_info_.pid);
                        // ここでcloseもしたい気持ちは若干ある ・//
                        dispatcher_.UnregisterConnectionReadEvent(context.cgi_info_.fd);
                    } else {
                        dispatcher_.UnregisterConnectionReadEvent(context.file_fd_);
                    }
                    const ServerConfig server_config = (config_.lower_bound(ServerConfigKey(socket_list_.GetPort(event.socket_fd), "")))->second;
                    this->SendErrorResponce(408, server_config, event.connection_fd);
                }
                // TODO(maitneel): socket の read側を監視対象から外す //
            } else if (event.event == kServerEventFail) {
                // TODO(maitneel): Do it;
                if (event_fd == event.connection_fd) {
                    fail_close++;
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
