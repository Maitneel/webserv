#include <unistd.h>
#include <poll.h>

#include <string>
#include <vector>
#include <map>
#include <set>
#include <utility>

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


FdManager::FdManager(const int &fd, const FdType &type) : fd_(fd), type_(type) {
}

FdManager::~FdManager() {
}

ReadWriteStatType FdManager::Read() {
    char buffer[BUFFER_SIZE];
    int read_size = 0;
    if (this->type_ == kConnection) {
        // recv;
    } else if (this->type_ == kFile) {
        read_size = read(this->fd_, &buffer, BUFFER_SIZE);
    }
    if (0 < read_size) {
        this->read_buffer_ += std::string(buffer, read_size);
    }
    if (read_size < 0) {
        return kFail;
    }
    return kSuccess;
}

ReadWriteStatType FdManager::Write() {
    if (this->writen_buffer_.length() == 0) {
        return kSuccess;
    }
    int writed_size;
    if (this->type_ == kConnection) {
        // send;
    } else if (this->type_ == kFile) {
        writed_size = write(this->fd_, this->writen_buffer_.c_str(), BUFFER_SIZE);
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
                FdManager &manager = this->fds_.at(it->fd);
                manager.Read();
                events.push_back(std::make_pair(it->fd, &manager));
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

// ------------------------------------------------------------------------ //
//                                                                          //
//                              ServerEventHandler                          //
//                                                                          //
// ------------------------------------------------------------------------ //

ServerEventHandler::ServerEventHandler() {
}

ServerEventHandler::~ServerEventHandler() {
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

void ServerEventHandler::UnregistorConnectionFd(const int &fd) {
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
            FdManager *manager = fd_events[i].second;
            if (this->socket_fds_.find(fd) == this->socket_fds_.end()) {
                int accept_fd = ft_accept(fd);
                this->fd_event_handler_.Register(accept_fd, kConnection);
                this->connection_fds_.insert(accept_fd);
                this->pairent_.insert(std::make_pair(accept_fd, fd));
                this->related_fd_[fd].insert(accept_fd);
            } else {
                int socket_fd = this->GetSocketFd(fd);
                int connection_fd = this->GetConnectionFd(fd);

                ConnectionEvent connection_event;
                connection_event.connection_fd = connection_fd;
                connection_event.socket_fd = socket_fd;
                connection_event.file_fd = -1;
                if (fd != connection_fd) {
                    connection_event.file_fd = fd;
                    connection_event.event = kReadableFile;
                } else {
                    connection_event.event = kReadableRequest;
                }
                connections.push_back(std::make_pair(fd, connection_event));
                is_wait_continue = false;
            }
        }
    } while (is_wait_continue);
    return connections;
}
