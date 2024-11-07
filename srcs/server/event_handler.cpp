#include <unistd.h>
#include <poll.h>
#include <sys/socket.h>

#include <string>
#include <vector>
#include <map>
#include <set>
#include <utility>
#include <algorithm>

#include "event_handler.hpp"

#include "poll_selector.hpp"

#define BUFFER_SIZE 1024

// TODO(maitneel): たぶんおそらくメイビー移動させる //
int ft_accept(int fd);

// ------------------------------------------------------------------------ //
//                                                                          //
//                                    FdManager                             //
//                                                                          //
// ------------------------------------------------------------------------ //


FdManager::FdManager(const int &fd, const FdType &type) : fd_(fd), type_(type), is_eof_(false) {
}

FdManager::~FdManager() {
}

ReadWriteStatType FdManager::Read() {
    char buffer[BUFFER_SIZE];
    int read_size = 0;
    if (this->type_ == kConnection) {
        read_size = recv(this->fd_, buffer, BUFFER_SIZE, 0);
    } else if (this->type_ == kFile) {
        read_size = read(this->fd_, &buffer, BUFFER_SIZE);
    }
    if (0 < read_size) {
        this->read_buffer_ += std::string(buffer, read_size);
    }
    if (read_size < 0) {
        return kFail;
    } else if (read_size == 0) {
        // read, recv が0を返した時、常にEOFなのか自信がない //
        this->is_eof_ = true;
        return kReturnedZero;
    }
    return kSuccess;
}

ReadWriteStatType FdManager::Write() {
    if (this->writen_buffer_.length() == 0) {
        return kSuccess;
    }
    int writed_size;
    if (this->type_ == kConnection) {
        writed_size = send(this->fd_, this->writen_buffer_.c_str(), std::min((std::string::size_type)(BUFFER_SIZE), this->writen_buffer_.length()), 0);
    } else if (this->type_ == kFile) {
        writed_size = write(this->fd_, this->writen_buffer_.c_str(), std::min((std::string::size_type)(BUFFER_SIZE), this->writen_buffer_.length()));
    }
    if (0 < writed_size) {
        this->writen_buffer_.erase(0, writed_size);
    }
    if (0 < this->writen_buffer_.size()) {
        return kContinue;
    } else if (writed_size == 0) {
        return kSuccess;
    } else {
        return kFail;
    }
}

const std::string &FdManager::get_read_buffer() {
    return this->read_buffer_;
}

void FdManager::add_writen_buffer(const std::string &src) {
    this->writen_buffer_ += src;
}

void FdManager::erase_read_buffer(const std::string::size_type &front, const std::string::size_type &len) {
    this->read_buffer_.erase(front, len);
}

const FdType &FdManager::get_type() const {
    return this->type_;
}


// ------------------------------------------------------------------------ //
//                                                                          //
//                                FdEventHandler                            //
//                                                                          //
// ------------------------------------------------------------------------ //

FdEventHandler::FdEventHandler() {
}

FdEventHandler::~FdEventHandler() {
}

void FdEventHandler::Register(const int &fd, const FdType &type) {
    this->fds_.insert(std::make_pair(fd, FdManager(fd, type)));
    struct pollfd pfd = {fd, POLLIN, 0};
    this->poll_fds_.push_back(pfd);
}

void FdEventHandler::Unregister(const int &fd) {
    if (this->fds_.find(fd) != this->fds_.end()) {
        this->fds_.erase(fd);
    }
    for (std::vector<pollfd>::iterator it = this->poll_fds_.begin(); it != this->poll_fds_.end(); it++) {
        if (it->fd == fd) {
            this->poll_fds_.erase(it);
            break;
        }
    }
}

std::vector<std::pair<int, FdManager *> > FdEventHandler::ReadBuffer() {
    std::vector<std::pair<int, FdManager *> > events;
    std::vector<pollfd>::iterator it;
    for (it = this->poll_fds_.begin(); it != this->poll_fds_.end(); it++) {
        if (it->revents != 0) {
            if (this->fds_.find(it->fd) != this->fds_.end()) {
                FdManager &fd_buffer = this->fds_.at(it->fd);
                fd_buffer.Read();
                events.push_back(std::make_pair(it->fd, &fd_buffer));
            } else {
                events.push_back(std::make_pair(it->fd, static_cast<FdManager *>(NULL)));
            }
        }
        it->revents = 0;
    }
    return events;
}

std::vector<std::pair<int, FdManager *> > FdEventHandler::Wait(int timeout) {
    bool should_continue_to_write;
    do {
        should_continue_to_write = false;
        for (std::map<int, FdManager>::iterator it = this->fds_.begin(); it != this->fds_.end(); it++) {
            if (it->second.Write() == kContinue) {
                should_continue_to_write = true;
            }
        }
        if (1 <= poll(this->poll_fds_.data(), this->poll_fds_.size(), 0)) {
            return ReadBuffer();
        }
    } while (should_continue_to_write);

    int ret = poll(this->poll_fds_.data(), this->poll_fds_.size(), timeout);
    if (ret < 0) {
        throw std::runtime_error("poll: failed");
    }
    if (ret == 0) {
        return std::vector<std::pair<int, FdManager *> > ();
    }
    return this->ReadBuffer();
}

FdManager *FdEventHandler::GetBuffer(const int &fd) {
    if (this->fds_.find(fd) != this->fds_.end()) {
        return &this->fds_.at(fd);
    }
    return NULL;
}

// ------------------------------------------------------------------------ //
//                                                                          //
//                              ServerEventHandler                          //
//                                                                          //
// ------------------------------------------------------------------------ //

ConnectionEvent::ConnectionEvent() {
}

ConnectionEvent::ConnectionEvent(
    const FdEventType &event_arg,
    FdManager *content_arg,
    const int &socket_fd_arg,
    const int &connection_fd_arg,
    const int &file_fd_arg
) : event(event_arg),  content(content_arg), socket_fd(socket_fd_arg), connection_fd(connection_fd_arg), file_fd(file_fd_arg) {
}

ServerEventHandler::ServerEventHandler() {
}

ServerEventHandler::~ServerEventHandler() {
}

ConnectionEvent ServerEventHandler::CreateConnectionEvent(const int &fd, FdManager *fd_buffer) {
    int socket_fd = this->GetSocketFd(fd);
    int connection_fd = this->GetConnectionFd(fd);
    int file_fd = -1;
    FdEventType event;

    if (fd != connection_fd) {
        file_fd = fd;
        event = kReadableFile;
    } else {
        event = kReadableRequest;
    }
    return ConnectionEvent(event, fd_buffer, socket_fd, connection_fd, file_fd);
}

void ServerEventHandler::RegistorNewConnection(const int &socket_fd) {
    int accept_fd = ft_accept(socket_fd);
    this->fd_event_handler_.Register(accept_fd, kConnection);
    this->connection_fds_.insert(accept_fd);
    this->pairent_.insert(std::make_pair(accept_fd, socket_fd));
    this->related_fd_[socket_fd].insert(accept_fd);
}

void ServerEventHandler::RegistorSocketFd(const int &fd) {
    this->socket_fds_.insert(fd);
}

void ServerEventHandler::RegistorFileFd(const int &fd, const int &connection_fd) {
    this->fd_event_handler_.Register(fd, kFile);
    if (0 <= connection_fd) {
        this->related_fd_.at(connection_fd).insert(fd);
        this->pairent_.insert(std::make_pair(fd, connection_fd));
    }
}

void ServerEventHandler::Unregistor(const int &fd) {
    for (std::set<int>::iterator it = this->related_fd_.at(fd).begin(); it != this->related_fd_.at(fd).end(); it++) {
        this->fd_event_handler_.Unregister(*it);
        this->pairent_.erase(*it);
    }
    related_fd_.erase(fd);
    this->fd_event_handler_.Unregister(fd);
    connection_fds_.erase(fd);
    this->related_fd_.at(this->pairent_.at(fd)).erase(fd);
    this->pairent_.erase(fd);
}

void ServerEventHandler::UnregistorConnectionFd(const int &fd) {
    if (this->connection_fds_.find(fd) != this->connection_fds_.end()) {
        this->Unregistor(fd);
    }
}

void ServerEventHandler::UnregistorFileFd(const int &fd) {
    if (this->socket_fds_.find(fd) == this->socket_fds_.end() && this->connection_fds_.find(fd) == this->connection_fds_.end()) {
        this->Unregistor(fd);
    }
}

void ServerEventHandler::UnRegistorConnectionOrFile(const int &fd) {
    if (this->socket_fds_.find(fd) == this->socket_fds_.end()) {
        this->Unregistor(fd);
    }
}

int ServerEventHandler::GetSocketFd(const int &fd) {
    if (this->socket_fds_.find(fd) != this->socket_fds_.end()) {
        return fd;
    } else if (this->connection_fds_.find(fd) != this->connection_fds_.end()) {
        return this->pairent_.at(fd);
    } else {
        return this->pairent_.at(this->pairent_.at(fd));
    }
}

int ServerEventHandler::GetConnectionFd(const int &fd) {
    if (this->socket_fds_.find(fd) != this->socket_fds_.end()) {
        return -1;
    } else if (this->connection_fds_.find(fd) != this->connection_fds_.end()) {
        return fd;
    } else {
        return this->pairent_.at(fd);
    }
}

std::vector<std::pair<int, ConnectionEvent> > ServerEventHandler::Wait(int timeout) {
    bool is_wait_continue;
    std::vector<std::pair<int, ConnectionEvent> > connections;
    do {
        is_wait_continue = true;
        std::vector<std::pair<int, FdManager *> > fd_events = this->fd_event_handler_.Wait(timeout);
        if (fd_events.size() == 0) {
            return connections;
        }
        for (size_t i = 0; i < fd_events.size(); i++) {
            int fd = fd_events[i].first;
            FdManager *fd_buffer = fd_events[i].second;
            if (fd_buffer != NULL && fd_buffer->is_eof_) {
                this->Unregistor(fd);
            } else if (this->socket_fds_.find(fd) == this->socket_fds_.end()) {
                this->RegistorNewConnection(fd);
            } else {
                connections.push_back(std::make_pair(fd, this->CreateConnectionEvent(fd, fd_buffer)));
                is_wait_continue = false;
            }
        }
    } while (is_wait_continue);
    return connections;
}

FdManager *ServerEventHandler::GetBuffer(const int &fd) {
    return this->fd_event_handler_.GetBuffer(fd);
}

/*
#include <sys/fcntl.h>
#include <iostream>
#define debug(s) std::cerr << #s << '\'' << (s) << '\'' << std::endl;
int main() {
    using namespace std;

    int of_fd = open("./hogehoge.txt", (O_RDWR | O_CREAT), 0777);
    int if_fd = open("./Makefile", O_RDONLY);

    debug(of_fd);
    debug(if_fd);

    FdEventHandler event_handler;
    event_handler.Register(of_fd, kFile);
    event_handler.Register(if_fd, kFile);
    event_handler.Register(STDIN_FILENO, kFile);
    FdManager *of_buffer = event_handler.GetBuffer(of_fd);
    FdManager *if_buffer = event_handler.GetBuffer(if_fd);
    FdManager *stdin_buffer = event_handler.GetBuffer(STDIN_FILENO);

    for (int i = 0; i < 10; i++) {
        vector<pair<int, FdManager*> > event_content =  event_handler.Wait(1000 * 1000);
        // of_buffer->add_writen_buffer("hogehogehoge\n");
        // cout << if_buffer->get_read_buffer() << endl;
        // cout << stdin_buffer->get_read_buffer() << endl;
        for (size_t j = 0; j < event_content.size(); j++) {
            if (event_content.at(j).second->is_eof_) {
                event_handler.Unregister(event_content.at(j).first);
            } else {
                cout << event_content.at(j).first << ' ' << event_content.at(j).second->get_read_buffer() << endl;
            }
        }
        
        cout << "=======================================================================" << endl;
    }

}
*/
