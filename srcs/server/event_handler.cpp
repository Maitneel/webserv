#include <unistd.h>
#include <poll.h>

#include <string>
#include <vector>
#include <map>
#include <set>

#include "event_handler.hpp"

#include "poll_selector.hpp"

#define BUFFER_SIZE 1024


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

void FdManager::erase_read_buffer(std::string::size_type &front, std::string::size_type &len) {
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

void FdEventHandler::RegisterSocket(const int &fd) {
    socket_fds_.insert(fd);
    this->poll_fds_.push_back({fd, POLLIN, 0});
}

void FdEventHandler::Register(const int &fd, FdManager *fd_manager) {
    this->fds_.insert(std::make_pair(fd, fd_manager));
    this->poll_fds_.push_back({fd, POLLIN, 0});
}

void FdEventHandler::Unregister(const int &fd) {
    if (this->socket_fds_.find(fd) != this->socket_fds_.end()) {
        this->socket_fds_.erase(fd);
    } else {
        this->fds_.erase(fd);
    }
    for (std::vector<pollfd>::iterator it = this->poll_fds_.begin(); it != this->poll_fds_.end(); it++) {
        if (it->fd == fd) {
            this->poll_fds_.erase(it);
            break;
        }
    }
}

std::vector<int> FdEventHandler::ReadBuffer() {
    std::vector<int> events;
    std::vector<pollfd>::iterator it;
    for (it = this->poll_fds_.begin(); it != this->poll_fds_.end(); it++) {
        if (it->revents != 0) {
            if (this->socket_fds_.find(it->fd) != this->socket_fds_.end()) {
                // TODO accept;
            } else {
                this->fds_.at(it->fd)->Read();
                events.push_back(it->fd);
            }
        }
        it->revents = 0;
    }
    return events;
}

std::vector<int> FdEventHandler::Wait(int timeout) {
    bool should_continue_to_write;
    do {
        should_continue_to_write = false;
        for (std::map<int, FdManager *>::iterator it = this->fds_.begin(); it != this->fds_.end(); it++) {
            if (it->second->Write() == kContinue) {
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
        return std::vector<int>();
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

std::vector<std::pair<int, FdEventType> > ServerEventHandler::Wait(int timeout) {

}