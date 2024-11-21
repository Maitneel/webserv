#include <unistd.h>
#include <poll.h>
#include <sys/socket.h>

#include <string>
#include <vector>
#include <map>
#include <set>
#include <utility>
#include <algorithm>

#include "event_dispatcher.hpp"

#include "poll_selector.hpp"

#define BUFFER_SIZE 1024

// TODO(maitneel): たぶんおそらくメイビー移動させる //
int ft_accept(int fd);

// ------------------------------------------------------------------------ //
//                                                                          //
//                                    FdManager                             //
//                                                                          //
// ------------------------------------------------------------------------ //


FdManager::FdManager(const int &fd, const FdType &type) : fd_(fd), type_(type), is_eof_(false), write_status_(kNoBuffer) {
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

// fdがnon-blocking であればブロックしないが、そうでなければブロックするのでnon-blockingである必要がある //
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
    } else if (writed_size == 0 || this->writen_buffer_.size() == 0) {
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
    this->write_status_ = kContinue;
}

void FdManager::erase_read_buffer(const std::string::size_type &front, const std::string::size_type &len) {
    this->read_buffer_.erase(front, len);
}

bool FdManager::HaveWriteableBuffer() {
    return (this->writen_buffer_.size() != 0);
}

const FdType &FdManager::get_type() const {
    return this->type_;
}

const bool &FdManager::IsEndedWrite() const {
    return (this->write_status_ == kSuccess);
}

// ------------------------------------------------------------------------ //
//                                                                          //
//                                FdEventDispatcher                            //
//                                                                          //
// ------------------------------------------------------------------------ //

FdEvent::FdEvent(const int &fd_arg, const FdEventType &event_arg, FdManager * content_arg) : fd_(fd_arg), event_(event_arg), content_(content_arg) {
}

FdEventDispatcher::FdEventDispatcher() {
}

FdEventDispatcher::~FdEventDispatcher() {
}

void FdEventDispatcher::Register(const int &fd, const FdType &type) {
    this->fds_.insert(std::make_pair(fd, FdManager(fd, type)));
    struct pollfd pfd = {fd, POLLIN, 0};
    this->poll_fds_.push_back(pfd);
}

void FdEventDispatcher::Unregister(const int &fd) {
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

std::vector<FdEvent> FdEventDispatcher::ReadBuffer() {
    std::vector<FdEvent> events;
    std::vector<pollfd>::iterator it;
    for (it = this->poll_fds_.begin(); it != this->poll_fds_.end(); it++) {
        if ((it->revents & POLLIN) == POLLIN) {
            if (this->fds_.find(it->fd) != this->fds_.end()) {
                FdManager &fd_buffer = this->fds_.at(it->fd);
                fd_buffer.Read();
                events.push_back(FdEvent(it->fd, kHaveReadableBuffer_, &fd_buffer));
            } else {
                events.push_back(FdEvent(it->fd, kChanged, static_cast<FdManager *>(NULL)));
            }
        }
        it->revents = 0;
    }
    return events;
}

std::vector<FdEvent> FdEventDispatcher::WriteBuffer() {
    std::vector<FdEvent> result_array;
    for (size_t i = 0; i < this->poll_fds_.size(); i++) {
        struct pollfd &processing = this->poll_fds_.at(i);
        const int fd = processing.fd;
        const short revents = processing.revents;

        if ((revents & POLLOUT) == POLLOUT) {
            ReadWriteStatType write_ret = this->fds_.at(fd).Write();
            if (write_ret == kSuccess) {
                result_array.push_back(FdEvent(fd, kWriteEnd_,  static_cast<FdManager *>(NULL)));
            } else if (write_ret == kFail) {
                result_array.push_back(FdEvent(fd, kFdEventFail_,  static_cast<FdManager *>(NULL)));
            }
        }
    }
    return result_array;
}

void FdEventDispatcher::UpdatePollEvents() {
    for (size_t i = 0; i < this->poll_fds_.size(); i++) {
        pollfd &processing = this->poll_fds_.at(i);
        const int fd = processing.fd;

        if (this->fds_.at(fd).HaveWriteableBuffer()) {
            processing.events = (POLLIN | POLLOUT);
        } else {
            processing.events = (POLLIN);
        }
    }
}

std::vector<FdEvent> FdEventDispatcher::Wait(int timeout) {
    std::vector<FdEvent> handled_readable_fd;
    std::vector<FdEvent> handled_write_fd;

    while (handled_readable_fd.size() == 0 && handled_write_fd.size() == 0) {
        this->UpdatePollEvents();
        int poll_ret = poll(this->poll_fds_.data(), this->poll_fds_.size(), timeout);
        if (poll_ret < 0) {
            throw std::runtime_error("poll: failed");
        }
        if (poll_ret == 0) {
            return std::vector<FdEvent> ();
        }
        handled_write_fd = this->WriteBuffer();
        handled_readable_fd = this->ReadBuffer();
    }
    // 少ない方から大きい方にマージすることで計算量が減る一般的なテク //
    if (handled_readable_fd.size() < handled_write_fd.size()) {
        std::swap(handled_readable_fd, handled_write_fd);
    }
    handled_readable_fd.insert(handled_readable_fd.begin(), handled_write_fd.begin(), handled_write_fd.end());
    return handled_readable_fd;
}

FdManager *FdEventDispatcher::GetBuffer(const int &fd) {
    if (this->fds_.find(fd) != this->fds_.end()) {
        return &this->fds_.at(fd);
    }
    return NULL;
}

// ------------------------------------------------------------------------ //
//                                                                          //
//                            ServerEventDispatcher                         //
//                                                                          //
// ------------------------------------------------------------------------ //

ConnectionEvent::ConnectionEvent() {
}

ConnectionEvent::ConnectionEvent(
    const ServerEventType &event_arg,
    FdManager *content_arg,
    const int &socket_fd_arg,
    const int &connection_fd_arg,
    const int &file_fd_arg
) : event(event_arg),  content(content_arg), socket_fd(socket_fd_arg), connection_fd(connection_fd_arg), file_fd(file_fd_arg) {
}

ServerEventDispatcher::ServerEventDispatcher() {
}

ServerEventDispatcher::~ServerEventDispatcher() {
}

ConnectionEvent ServerEventDispatcher::CreateConnectionEvent(const int &fd, const FdEventType &fd_event, FdManager *fd_buffer) {
    int socket_fd = this->GetSocketFd(fd);
    int connection_fd = this->GetConnectionFd(fd);
    int file_fd = -1;
    ServerEventType event;

    if (fd_event == kWriteEnd_) {
        if (fd != connection_fd) {
            file_fd = fd;
            event = kFileWriteEnd_;
        } else {
            event = kResponceWriteEnd_;
        }
    } else {
        if (fd != connection_fd) {
            file_fd = fd;
            event = kReadableFile;
        } else {
            event = kReadableRequest;
        }
    }
    return ConnectionEvent(event, fd_buffer, socket_fd, connection_fd, file_fd);
}

void ServerEventDispatcher::RegistorNewConnection(const int &socket_fd) {
    int accept_fd = ft_accept(socket_fd);
    this->fd_event_dispatcher_.Register(accept_fd, kConnection);
    this->connection_fds_.insert(accept_fd);
    this->pairent_.insert(std::make_pair(accept_fd, socket_fd));
    this->related_fd_[socket_fd].insert(accept_fd);
}

void ServerEventDispatcher::RegistorSocketFd(const int &fd) {
    this->socket_fds_.insert(fd);
}

void ServerEventDispatcher::RegistorFileFd(const int &fd, const int &connection_fd) {
    this->fd_event_dispatcher_.Register(fd, kFile);
    if (0 <= connection_fd) {
        this->related_fd_.at(connection_fd).insert(fd);
        this->pairent_.insert(std::make_pair(fd, connection_fd));
    }
}

void ServerEventDispatcher::Unregistor(const int &fd) {
    for (std::set<int>::iterator it = this->related_fd_.at(fd).begin(); it != this->related_fd_.at(fd).end(); it++) {
        this->fd_event_dispatcher_.Unregister(*it);
        this->pairent_.erase(*it);
    }
    related_fd_.erase(fd);
    this->fd_event_dispatcher_.Unregister(fd);
    connection_fds_.erase(fd);
    this->related_fd_.at(this->pairent_.at(fd)).erase(fd);
    this->pairent_.erase(fd);
}

void ServerEventDispatcher::UnregistorConnectionFd(const int &fd) {
    if (this->connection_fds_.find(fd) != this->connection_fds_.end()) {
        this->Unregistor(fd);
    }
}

void ServerEventDispatcher::UnregistorFileFd(const int &fd) {
    if (this->socket_fds_.find(fd) == this->socket_fds_.end() && this->connection_fds_.find(fd) == this->connection_fds_.end()) {
        this->Unregistor(fd);
    }
}

void ServerEventDispatcher::UnRegistorConnectionOrFile(const int &fd) {
    if (this->socket_fds_.find(fd) == this->socket_fds_.end()) {
        this->Unregistor(fd);
    }
}

int ServerEventDispatcher::GetSocketFd(const int &fd) {
    if (this->socket_fds_.find(fd) != this->socket_fds_.end()) {
        return fd;
    } else if (this->connection_fds_.find(fd) != this->connection_fds_.end()) {
        return this->pairent_.at(fd);
    } else {
        return this->pairent_.at(this->pairent_.at(fd));
    }
}

int ServerEventDispatcher::GetConnectionFd(const int &fd) {
    if (this->socket_fds_.find(fd) != this->socket_fds_.end()) {
        return -1;
    } else if (this->connection_fds_.find(fd) != this->connection_fds_.end()) {
        return fd;
    } else {
        return this->pairent_.at(fd);
    }
}

std::vector<std::pair<int, ConnectionEvent> > ServerEventDispatcher::Wait(int timeout) {
    bool is_wait_continue;
    std::vector<std::pair<int, ConnectionEvent> > connections;
    do {
        std::vector<FdEvent> fd_events = this->fd_event_dispatcher_.Wait(timeout);
        if (fd_events.size() == 0) {
            return connections;
        }
        for (size_t i = 0; i < fd_events.size(); i++) {
            int fd = fd_events[i].fd_;
            FdManager *fd_buffer = fd_events[i].content_;
            if (fd_events[i].event_ == kHaveReadableBuffer_) {
                connections.push_back(std::make_pair(fd, this->CreateConnectionEvent(fd, fd_events[i].event_, fd_buffer)));
            } else if (fd_events[i].event_ == kChanged) {
                this->RegistorNewConnection(fd);
            } else if (fd_events[i].event_ == kWriteEnd_) {
                connections.push_back(std::make_pair(fd, this->CreateConnectionEvent(fd, fd_events[i].event_, fd_buffer)));
            }

            // TODO(maitneel):  なんでunregustirしてるか全然わかんないけど、なんかしてるから要検証 //
            // if (fd_buffer != NULL && fd_buffer->is_eof_) {
            //    this->Unregistor(fd);
            // }
        }
    } while (connections.size() == 0);
    return connections;
}

FdManager *ServerEventDispatcher::GetBuffer(const int &fd) {
    return this->fd_event_dispatcher_.GetBuffer(fd);
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

    FdEventDispatcher event_dispatcher;
    event_dispatcher.Register(of_fd, kFile);
    event_dispatcher.Register(if_fd, kFile);
    event_dispatcher.Register(STDIN_FILENO, kFile);
    FdManager *of_buffer = event_dispatcher.GetBuffer(of_fd);
    FdManager *if_buffer = event_dispatcher.GetBuffer(if_fd);
    FdManager *stdin_buffer = event_dispatcher.GetBuffer(STDIN_FILENO);

    for (int i = 0; i < 10; i++) {
        vector<FdEvent> event_content =  event_dispatcher.Wait(1000 * 1000);
        // of_buffer->add_writen_buffer("hogehogehoge\n");
        // cout << if_buffer->get_read_buffer() << endl;
        // cout << stdin_buffer->get_read_buffer() << endl;
        for (size_t j = 0; j < event_content.size(); j++) {
            if (event_content.at(j).content_->is_eof_) {
                event_dispatcher.Unregister(event_content.at(j).fd_);
            } else {
                cout << "[fd, event, buffer]: [" << event_content.at(j).fd_ << ", " << event_content.at(j).event_ << ", '" << event_content.at(j).content_->get_read_buffer() << "']" << endl;
            }
        }
        
        cout << "=======================================================================" << endl;
    }

}
// */


/*
// how to test this tests is:
// g++ ./temp/delay_read.cpp
// ./webserv | ./a.out

#include <iostream>
using namespace std;
int main() {
    FdEventDispatcher fde;
    fde.Register(STDOUT_FILENO, kFile);
    for (int i = 0; i < 105; i++) {
        fde.GetBuffer(STDOUT_FILENO)->add_writen_buffer("0123456789");
    }
    while (fde.GetBuffer(STDOUT_FILENO)->HaveWriteableBuffer()) {
        vector<FdEvent> event = fde.Wait(2000);
        cerr << event.size() << endl;
        for (size_t i = 0; i < event.size(); i++) {
            cerr << "[fd, event] : [" << event[i].fd_ << ", " << event[i].event_ << "]" << endl;
        }
        cerr << "-------------------------------------------------------------" << endl;
    }
    cerr << "webserv end" << endl;
}

//  */
