#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/time.h>

#include <climits>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <set>
#include <utility>
#include <algorithm>

#include <iostream>
#include <iomanip>
using std::cerr;
using std::endl;

#include "event_dispatcher.hpp"
#include "extend_stdlib.hpp"

#define BUFFER_SIZE 1048576
#define MAX_NUMBER_OF_CONNECTION 10

int ft_accept(int fd);

long get_usec() {
    struct timeval tv;
    if (gettimeofday(&tv, NULL)) {
        return -1;
    }
    return (tv.tv_sec * SEC_PER_USEC + tv.tv_usec);
}

// ------------------------------------------------------------------------ //
//                                                                          //
//                                    FdManager                             //
//                                                                          //
// ------------------------------------------------------------------------ //

// C++98においてatomicな型は存在しないっぽい(Cのatomic型もC11からしかなさそう)のでこの辺で妥協したい //
static volatile int recived_signal = 0;
static void signal_handler(int sigid) {
    cerr << "recive signal" << endl;
    recived_signal = sigid;
}

FdManager::FdManager(const int &fd, const FdType &type) : fd_(fd), type_(type), write_status_(kNoBuffer), write_head_(0) {
}

FdManager::~FdManager() {
}

ReadWriteStatType FdManager::Read() {
    char buffer[BUFFER_SIZE];
    ssize_t read_size = 0;
    if (this->type_ == kSocket) {
        return kDidNotRead;
    }
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
        return kReturnedZero;
    }
    return kSuccess;
}

// fdがnon-blocking であればブロックしないが、そうでなければブロックするのでnon-blockingである必要がある //
ReadWriteStatType FdManager::Write() {
    if (BUFFER_ERASE_LENGTH < this->write_head_) {
        this->writen_buffer_.erase(0, this->write_head_);
        this->write_head_ = 0;
    }
    if (this->writen_buffer_.length() - this->write_head_ == 0) {
        return kSuccess;
    }
    ssize_t writed_size = 0;
    if (this->type_ == kConnection) {
        writed_size = send(this->fd_, this->writen_buffer_.c_str() + this->write_head_, std::min((std::string::size_type)(BUFFER_SIZE), this->writen_buffer_.length() - this->write_head_), 0);
    } else if (this->type_ == kFile) {
        writed_size = write(this->fd_, this->writen_buffer_.c_str() + this->write_head_, std::min((std::string::size_type)(BUFFER_SIZE), this->writen_buffer_.length() - this->write_head_));
    }

    if (0 < writed_size) {
        this->write_head_ += writed_size;
    }
    if (writed_size == -1) {
        return kFail;
    } else if (0 < this->writen_buffer_.size() - this->write_head_) {
        return kContinue;
    } else if (writed_size == 0 || this->writen_buffer_.size() - write_head_ == 0) {
        return kSuccess;
    } else {
        return kFail;
    }
}

const std::string &FdManager::get_read_buffer() const {
    return this->read_buffer_;
}

void FdManager::add_writen_buffer(const std::string &src) {
    this->writen_buffer_ += src;
    this->write_status_ = kContinue;
}

void FdManager::erase_read_buffer(const std::string::size_type &front, const std::string::size_type &len) {
    this->read_buffer_.erase(front, len);
}

bool FdManager::IsEmptyWritebleBuffer() {
    return (this->writen_buffer_.size() - this->write_head_ == 0);
}

const FdType &FdManager::get_type() const {
    return this->type_;
}

// ------------------------------------------------------------------------ //
//                                                                          //
//                                FdEventDispatcher                            //
//                                                                          //
// ------------------------------------------------------------------------ //

FdEvent::FdEvent(const int &fd_arg, const FdEventType &event_arg) : fd_(fd_arg), event_(event_arg) {
}

const std::string FdEventDispatcher::empty_string_ = "";

FdEventDispatcher::FdEventDispatcher() {
    signal(SIGCHLD, signal_handler);
    // なんかやっといた方がいいっぽい //
    signal(SIGPIPE, SIG_IGN);
}

FdEventDispatcher::~FdEventDispatcher() {
    signal(SIGCHLD, SIG_DFL);
    signal(SIGPIPE, SIG_DFL);
}

void FdEventDispatcher::Register(const int &fd, const FdType &type) {
    this->fds_.insert(std::make_pair(fd, FdManager(fd, type)));
    this->registerd_read_fds_.insert(fd);
    struct pollfd pfd = {fd, POLLIN, 0};
    this->poll_fds_.push_back(pfd);
}

void FdEventDispatcher::Unregister(const int &fd) {
    if (this->fds_.find(fd) != this->fds_.end()) {
        this->fds_.erase(fd);
    }
    if (registerd_read_fds_.find(fd) != registerd_read_fds_.end()) {
        registerd_read_fds_.erase(fd);
    }
    for (std::vector<pollfd>::iterator it = this->poll_fds_.begin(); it != this->poll_fds_.end(); it++) {
        if (it->fd == fd) {
            this->poll_fds_.erase(it);
            break;
        }
    }
}

void FdEventDispatcher::UnregisterReadEvent(const int &fd) {
    registerd_read_fds_.erase(fd);
}

std::multimap<int, FdEvent> FdEventDispatcher::ReadBuffer() {
    std::multimap<int, FdEvent> events;
    std::vector<pollfd>::iterator it;
    for (it = this->poll_fds_.begin(); it != this->poll_fds_.end(); it++) {
        if ((it->revents & POLLIN) == POLLIN) {
            if (this->fds_.find(it->fd) != this->fds_.end()) {
                FdManager &fd_buffer = map_at(&this->fds_, it->fd);
                ReadWriteStatType stat = fd_buffer.Read();
                FdEventType event_type;

                if (stat == kReturnedZero) {
                    event_type = kEOF;
                } else if (stat == kDidNotRead) {
                    event_type = kChanged;
                } else if (stat == kFail) {
                    event_type = kFdEventFail;
                } else {
                    event_type = kHaveReadableBuffer;
                }
                events.insert(std::make_pair(it->fd, FdEvent(it->fd, event_type)));
            } else {
                events.insert(std::make_pair(it->fd, FdEvent(it->fd, kChanged)));
            }
        }
    }
    return events;
}

std::multimap<int, FdEvent> FdEventDispatcher::WriteBuffer() {
    std::multimap<int, FdEvent> result_array;
    for (size_t i = 0; i < this->poll_fds_.size(); i++) {
        struct pollfd &processing = this->poll_fds_.at(i);
        const int fd = processing.fd;
        const short &revents = processing.revents;
        const short &events = processing.events;

        if ((revents & POLLOUT) == POLLOUT && !(revents & (~events))) {
            ReadWriteStatType write_ret = map_at(&this->fds_, fd).Write();
            if (write_ret == kSuccess) {
                result_array.insert(std::make_pair(fd, FdEvent(fd, kWriteEnd)));
            } else if (write_ret == kFail) {
                result_array.insert(std::make_pair(fd, FdEvent(fd, kFdEventFail)));
            }
        }
    }
    return result_array;
}

std::multimap<int, FdEvent> FdEventDispatcher::GetErrorFds() {
    std::multimap<int, FdEvent> error_fds;
    for (size_t i = 0; i < this->poll_fds_.size(); i++) {
        struct pollfd &processing = this->poll_fds_.at(i);
        const int &fd = processing.fd;
        const short &events = processing.events;
        const short &revents = processing.revents;

        if (revents & (~events)) {
            error_fds.insert(std::make_pair(fd, FdEvent(fd, kFdEventFail)));
        }
    }
    return error_fds;
}


void FdEventDispatcher::UpdatePollEvents() {
    for (size_t i = 0; i < this->poll_fds_.size(); i++) {
        pollfd &processing = this->poll_fds_.at(i);
        const int fd = processing.fd;

        if (!map_at(&this->fds_, fd).IsEmptyWritebleBuffer()) {
            if (registerd_read_fds_.find(fd) != registerd_read_fds_.end()) {
                processing.events = (POLLIN | POLLOUT);
            } else {
                processing.events = (POLLOUT);
            }
        } else {
            if (registerd_read_fds_.find(fd) != registerd_read_fds_.end()) {
                processing.events = (POLLIN);
            } else {
                processing.events = (0);
            }
        }
        processing.revents = 0;
    }
}

std::multimap<int, FdEvent> FdEventDispatcher::MergeEvents(const std::multimap<int, FdEvent> &read_event, const std::multimap<int, FdEvent> &write_events, const std::multimap<int, FdEvent> &error_events) {
    std::multimap<int, FdEvent> events(read_event);

    for (std::multimap<int, FdEvent>::const_iterator it = write_events.begin(); it != write_events.end(); it++) {
        events.insert(*it);
    }
    for (std::multimap<int, FdEvent>::const_iterator it = error_events.begin(); it != error_events.end(); it++) {
        events.insert(*it);
    }
    return events;
}

std::multimap<int, FdEvent> FdEventDispatcher::Wait(int timeout) {
    if (poll_fds_.size() == 0) {
        return std::multimap<int, FdEvent>();
    }

    std::multimap<int, FdEvent> handled_readable_fd;
    std::multimap<int, FdEvent> handled_write_fd;
    std::multimap<int, FdEvent> handled_error_fd;

    if (recived_signal != 0) {
        recived_signal = 0;
        throw SignalDelivered(SIGCHLD);
    }
        while (handled_readable_fd.size() == 0 && handled_write_fd.size() == 0 && handled_error_fd.size() == 0) {
            this->UpdatePollEvents();
            int poll_ret = poll(this->poll_fds_.data(), this->poll_fds_.size(), timeout);
            if (recived_signal != 0) {
                recived_signal = 0;
                throw SignalDelivered(SIGCHLD);
            }
            if (poll_ret < 0) {
                throw std::runtime_error("poll: failed");
            }
            if (poll_ret == 0) {
                return std::multimap<int, FdEvent> ();
            }
            handled_write_fd = this->WriteBuffer();
            handled_readable_fd = this->ReadBuffer();
            handled_error_fd = this->GetErrorFds();
        }
    return this->MergeEvents(handled_readable_fd, handled_write_fd, handled_error_fd);
}

const std::string &FdEventDispatcher::get_read_buffer(const int &fd) const {
    const std::map<int, FdManager>::const_iterator target = this->fds_.find(fd);
    if (target != this->fds_.end()) {
        return target->second.get_read_buffer();
    }
    // 何かしら throw するべきかも //
    return this->empty_string_;
}

void FdEventDispatcher::add_writen_buffer(const int &fd, const std::string &src) {
    const std::map<int, FdManager>::iterator target = this->fds_.find(fd);
    if (target != this->fds_.end()) {
        return target->second.add_writen_buffer(src);
    }
}

void FdEventDispatcher::erase_read_buffer(const int &fd, const std::string::size_type &front, const std::string::size_type &len) {
    const std::map<int, FdManager>::iterator target = this->fds_.find(fd);
    if (target != this->fds_.end()) {
        return target->second.erase_read_buffer(front, len);
    }
}

bool FdEventDispatcher::IsEmptyWritebleBuffer(const int &fd) {
    const std::map<int, FdManager>::iterator target = this->fds_.find(fd);
    if (target != this->fds_.end()) {
        return target->second.IsEmptyWritebleBuffer();
    }
    // ここも throw するべきかも  //
    return true;
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
    const int &socket_fd_arg,
    const int &connection_fd_arg,
    const int &file_fd_arg
) : event(event_arg), socket_fd(socket_fd_arg), connection_fd(connection_fd_arg), file_fd(file_fd_arg) {
}

const std::set<AnyFdType> RelatedFds::empty_any_fd_type_set_;

RelatedFds::RelatedFds() {
}

RelatedFds::~RelatedFds() {
}

int RelatedFds::GetPairentSocket(const AnyFdType &fd) {
    if (this->socket_fds_.find(fd) != this->socket_fds_.end()) {
        return fd;
    }
    if (this->pairent_socket_.find(fd) != this->pairent_socket_.end()) {
        return this->pairent_socket_.find(fd)->second;
    }
    return NON_EXIST_FD;
}

int RelatedFds::GetPairentConnection(const FileFdType &fd) {
    if (this->connection_fds_.find(fd) != this->connection_fds_.end()) {
        return fd;
    }
    if (this->pairent_connection_.find(fd) != this->pairent_connection_.end()) {
        return this->pairent_connection_.find(fd)->second;
    }
    return NON_EXIST_FD;
}

const std::set<AnyFdType> &RelatedFds::GetSocketChildren(const SocketFdType &fd) {
    return this->socket_children_.find(fd)->second;
}

const std::set<FileFdType> &RelatedFds::GetConnectionChildren(const ConnectionFdType &fd) {
    return this->connection_childlen_.find(fd)->second;
}

void RelatedFds::RegisterSocketFd(const SocketFdType &socket_fd) {
    this->registerd_fds_.insert(socket_fd);
    this->socket_fds_.insert(socket_fd);
    this->fd_type_.insert(std::make_pair(socket_fd, kSocket));
    this->socket_children_.insert(std::make_pair(socket_fd, std::set<AnyFdType>()));
}

void RelatedFds::RegisterConnectionFd(const ConnectionFdType &connection_fd, const SocketFdType &socket_fd) {
    this->registerd_fds_.insert(connection_fd);
    this->connection_fds_.insert(connection_fd);
    this->fd_type_.insert(std::make_pair(connection_fd, kConnection));
    this->pairent_socket_.insert(std::make_pair(connection_fd, socket_fd));
    map_at(&this->socket_children_, socket_fd).insert(connection_fd);
    this->connection_childlen_.insert(std::make_pair(connection_fd, std::set<FileFdType>()));
}

void RelatedFds::RegisterFileFd(const FileFdType &file_fd, const ConnectionFdType &connection_fd) {
    const int socket_fd = this->GetPairentSocket(connection_fd);
    if (socket_fd == NON_EXIST_FD) {
        throw std::runtime_error("unregisterd socket fd");
    }

    this->registerd_fds_.insert(file_fd);
    this->file_fds_.insert(file_fd);
    this->fd_type_.insert(std::make_pair(file_fd, kFile));
    this->pairent_socket_.insert(std::make_pair(file_fd, socket_fd));
    map_at(&this->socket_children_, socket_fd).insert(file_fd);
    this->pairent_connection_.insert(std::make_pair(file_fd, connection_fd));
    map_at(&this->connection_childlen_, connection_fd).insert(file_fd);
}

void RelatedFds::UnregisterSocketFd(const SocketFdType &socket_fd) {
    this->registerd_fds_.erase(socket_fd);
    this->socket_fds_.erase(socket_fd);
    this->fd_type_.erase(socket_fd);
    std::set<AnyFdType> children = map_at(&this->socket_children_, socket_fd);
    for (std::set<AnyFdType>::iterator it = children.begin(); it != children.end(); it++) {
        if (this->connection_fds_.find(*it) != this->connection_fds_.end()) {
            this->UnregisterConnectionFd(*it);
        } else if (this->file_fds_.find(*it) != this->file_fds_.end()) {
            this->UnregisterFileFd(*it);
        }
    }
    this->socket_children_.erase(socket_fd);
}

void RelatedFds::UnregisterConnectionFd(const ConnectionFdType &connection_fd) {
    const SocketFdType socket_fd = map_at(&this->pairent_socket_, connection_fd);

    this->registerd_fds_.erase(connection_fd);
    this->connection_fds_.erase(connection_fd);
    this->fd_type_.erase(connection_fd);
    this->pairent_socket_.erase(connection_fd);
    map_at(&this->socket_children_, socket_fd).erase(connection_fd);
    std::set<AnyFdType> children = map_at(&this->connection_childlen_, connection_fd);
    for (std::set<AnyFdType>::iterator it = children.begin(); it != children.end(); it++) {
        this->UnregisterFileFd(*it);
    }
    this->connection_childlen_.erase(connection_fd);
}

void RelatedFds::UnregisterFileFd(const FileFdType &file_fd) {
    const SocketFdType socket_fd = map_at(&this->pairent_socket_, file_fd);
    const ConnectionFdType connection_fd = map_at(&this->pairent_connection_, file_fd);

    this->registerd_fds_.erase(file_fd);
    this->file_fds_.erase(file_fd);
    this->fd_type_.erase(file_fd);
    map_at(&this->socket_children_, socket_fd).erase(file_fd);
    this->pairent_socket_.erase(file_fd);
    map_at(&this->connection_childlen_, connection_fd).erase(file_fd);
    this->pairent_connection_.erase(file_fd);
}

FdType RelatedFds::GetType(const AnyFdType &fd) const {
    if (this->fd_type_.find(fd) == this->fd_type_.end()) {
        return kUnknownFd;
    }
    return this->fd_type_.find(fd)->second;
}

const std::set<AnyFdType> &RelatedFds::GetChildrenFd(const int &fd) {
    if (this->fd_type_.find(fd) == this->fd_type_.end()) {
        throw std::runtime_error("unregisterd");
    }
    FdType type = this->fd_type_.find(fd)->second;
    if (type == kSocket) {
        return this->socket_children_.find(fd)->second;
    } else if (type == kConnection) {
        return this->connection_childlen_.find(fd)->second;
    }
    return empty_any_fd_type_set_;
}

bool RelatedFds::IsRegistered(const AnyFdType &fd) {
    return (registerd_fds_.find(fd) != registerd_fds_.end());
}

/*
#include <iostream>
void RelatedFds::print() {
    std::cerr << "socket_fds      : ";
    for (auto i = this->socket_fds_.begin(); i != this->socket_fds_.end(); i++) {
        std::cerr << *i << ", ";
    }
    std::cerr << std::endl;
    
    std::cerr << "connection_fds_ : ";
    for (auto i = this->connection_fds_.begin(); i != this->connection_fds_.end(); i++) {
        std::cerr << *i << ", ";
    }
    std::cerr << std::endl;

    std::cerr << "file_fds_       : ";
    for (auto i = this->file_fds_.begin(); i != this->file_fds_.end(); i++) {
        std::cerr << *i << ", ";
    }
    std::cerr << std::endl;

    std::cerr << "pairent_socket" << std::endl;
    for (auto i = this->pairent_socket_.begin(); i != this->pairent_socket_.end(); i++) {
        std::cerr << i->first << ": " << i->second << std::endl;
    }

    std::cerr << "pairent_connection" << std::endl;
    for (auto i = this->pairent_connection_.begin(); i != this->pairent_connection_.end(); i++) {
        std::cerr << i->first << ": " << i->second << std::endl;
    }

    for (auto i = this->socket_children_.begin(); i != this->socket_children_.end(); i++) {
        std::cerr << "socket_children : " << i->first << " -------------------" << std::endl;
        for (auto j = i->second.begin(); j != i->second.end(); j++) {
            std::cerr << *j << ", ";
        }
        std::cerr << std::endl;
    }

    for (auto i = this->connection_childlen_.begin(); i != this->connection_childlen_.end(); i++) {
        std::cerr << "connection_children : " << i->first << " -------------------" << std::endl;
        for (auto j = i->second.begin(); j != i->second.end(); j++) {
            std::cerr << *j << ", ";
        }
        std::cerr << std::endl;
    }
}
// */

ServerEventDispatcher::ServerEventDispatcher() {
}

ServerEventDispatcher::~ServerEventDispatcher() {
}

ConnectionEvent ServerEventDispatcher::CreateConnectionEvent(const int &fd, const ServerEventType &server_event) {
    int socket_fd = this->registerd_fds_.GetPairentSocket(fd);
    int connection_fd = this->registerd_fds_.GetPairentConnection(fd);
    int file_fd = NON_EXIST_FD;

    return ConnectionEvent(server_event, socket_fd, connection_fd, file_fd);
}

ConnectionEvent ServerEventDispatcher::CreateConnectionEvent(const int &fd, const FdEventType &fd_event) {
    int socket_fd = this->registerd_fds_.GetPairentSocket(fd);
    int connection_fd = this->registerd_fds_.GetPairentConnection(fd);
    int file_fd = NON_EXIST_FD;
    ServerEventType event = kUnknownEvent;

    if (fd == connection_fd) {
        if (fd_event == kFdEventTypeUndefined) {
            // Nothing to do;
        } else if (fd_event == kHaveReadableBuffer) {
            event = kReadableRequest;
        } else if (fd_event == kEOF) {
            event = kRequestEndOfReaded;
        } else if (fd_event == kChanged) {
            // Nothing to do;
        } else if (fd_event == kWriteEnd) {
            event = kresponseWriteEnd;
        } else if (fd_event == kFdEventFail) {
            event = kServerEventFail;
        }
    } else if (fd != connection_fd) {
        file_fd = fd;
        if (fd_event == kFdEventTypeUndefined) {
            // Nothing to do;
        } else if (fd_event == kHaveReadableBuffer) {
            event = kReadableFile;
        } else if (fd_event == kEOF) {
            event = kFileEndOfRead;
        } else if (fd_event == kChanged) {
            // Nothing to do;
        } else if (fd_event == kWriteEnd) {
            event = kFileWriteEnd;
        } else if (fd_event == kFdEventFail) {
            event = kServerEventFail;
        }
    }

    return ConnectionEvent(event, socket_fd, connection_fd, file_fd);
}

void ServerEventDispatcher::MergeDuplicateFd(std::multimap<int, FdEvent> *events) {
    for (std::multimap<int, FdEvent>::iterator it = events->begin(); it != events->end(); it++) {
        const int fd = it->first;
        if (events->count(fd) == 1) {
            continue;
        }

        std::multimap<int, FdEvent>::iterator read_event = events->end();
        std::multimap<int, FdEvent>::iterator error_event = events->end();
        for (std::multimap<int, FdEvent>::iterator dup_fd_it = it; dup_fd_it->first == fd; dup_fd_it++) {
            int event = dup_fd_it->second.event_;
            if (event == kHaveReadableBuffer || event == kEOF) {
                read_event = dup_fd_it;
            } else if (event == kFdEventFail) {
                error_event = dup_fd_it;
            }
        }
        if (read_event != events->end() && error_event != events->end()) {
            const FdType &type = this->registerd_fds_.GetType(fd);
            if (type == kFile) {
                events->erase(error_event);
            } else {
                events->erase(read_event);
            }
        }
        it = events->upper_bound(fd);
        it--;
    }
}


std::set<int> ServerEventDispatcher::CheckTimeout() {
    std::set<int> timeout;
    const long now = get_usec();
    for (std::map<int, long>::iterator it = continue_connection_until_.begin(); it != continue_connection_until_.end(); it++) {
        if (it->second < now) {
            timeout.insert(it->first);
            // 2回以上timeoutイベントが走るとめんどくさいので雑な処理 //
            it->second = LONG_MAX;
        }
    }
    return timeout;
}
void ServerEventDispatcher::OverrideTimeoutEvent(std::multimap<int, ConnectionEvent> *events) {
    std::set<int> timeout_fds = CheckTimeout();
    for (std::set<int>::iterator it = timeout_fds.begin(); it != timeout_fds.end(); it++) {
        if (events->find(*it) != events->end()) {
            events->erase(*it);
        }
        events->insert(std::make_pair(*it, CreateConnectionEvent(*it, kTimeout)));
    }
}

int ServerEventDispatcher::CalcWaitTime(int *timeout) {
    if (continue_connection_until_.size() == 0) {
        return *timeout;
    }
    if (*timeout < 0) {
        return (TIMEOUT_LENGTH_USEC / (SEC_PER_USEC / SEC_PER_MS));
    }
    int result = 0;
    if (*timeout < TIMEOUT_LENGTH_USEC) {
        result = *timeout;
        *timeout = 0;
    } else {
        result = TIMEOUT_LENGTH_USEC;
        *timeout -= TIMEOUT_LENGTH_USEC;
    }
    return result;
}

int accept_count = 0;

void ServerEventDispatcher::RegisterNewConnection(const int &socket_fd) {
    int connection_fd;
    if (this->registerd_fds_.connection_fds_.size() < MAX_NUMBER_OF_CONNECTION) {
        connection_fd = ft_accept(socket_fd);
        this->times_.insert(std::make_pair(connection_fd, get_usec()));
        accept_count++;
        // cerr << "accept: " << socket_fd << ' ' << connection_fd << ", count: " << accept_count << endl;
        this->continue_connection_until_.insert(std::make_pair(connection_fd, get_usec() + TIMEOUT_LENGTH_USEC));
        this->fd_event_dispatcher_.Register(connection_fd, kConnection);
        this->registerd_fds_.RegisterConnectionFd(connection_fd, socket_fd);
    }
}

void ServerEventDispatcher::RegisterSocketFd(const int &socket_fd) {
    if (!this->registerd_fds_.IsRegistered(socket_fd)) {
        this->registerd_fds_.RegisterSocketFd(socket_fd);
        this->fd_event_dispatcher_.Register(socket_fd, kSocket);
    }
}

void ServerEventDispatcher::RegisterFileFd(const int &file_fd, const int &connection_fd) {
    this->fd_event_dispatcher_.Register(file_fd, kFile);
    if (0 <= connection_fd) {
        this->registerd_fds_.RegisterFileFd(file_fd, connection_fd);
    }
}

void ServerEventDispatcher::UnregisterConnectionFd(const int &connection_fd) {
    if (this->registerd_fds_.GetType(connection_fd) != kConnection) {
        return;
    }
    std::map<int, long>::iterator it = times_.find(connection_fd);
    if (!(get_usec() - it->second / SEC_PER_USEC)) {
        cerr << "time: " << std::setw(2) << it->first << ": " <<  get_usec() - it->second << endl;
    }
    times_.erase(it);

    const std::set<int> children = this->registerd_fds_.GetChildrenFd(connection_fd);
    for (std::set<int>::const_iterator it = children.begin(); it != children.end(); it++) {
        this->fd_event_dispatcher_.Unregister(*it);
        this->registerd_fds_.UnregisterFileFd(*it);
    }
    this->fd_event_dispatcher_.Unregister(connection_fd);
    this->registerd_fds_.UnregisterConnectionFd(connection_fd);
    this->continue_connection_until_.erase(connection_fd);
}

void ServerEventDispatcher::UnregisterConnectionReadEvent(const int &fd) {
    fd_event_dispatcher_.UnregisterReadEvent(fd);
}

void ServerEventDispatcher::UnregisterFileFd(const int &file_fd) {
    if (this->registerd_fds_.GetType(file_fd) != kFile) {
        return;
    }
    this->fd_event_dispatcher_.Unregister(file_fd);
    this->registerd_fds_.UnregisterFileFd(file_fd);
}

void ServerEventDispatcher::UnregisterWithClose(const int &fd) {
    FdType type = registerd_fds_.GetType(fd);

    if (type == kUnknownFd) {
        return;
    } else if (type == kConnection) {
        std::set<int> children_fds = registerd_fds_.GetConnectionChildren(fd);
        for (std::set<int>::iterator it = children_fds.begin(); it != children_fds.end(); it++) {
            this->UnregisterWithClose(*it);
        }
        this->UnregisterConnectionFd(fd);
    } else if (type == kFile) {
        this->UnregisterFileFd(fd);
    }
    close(fd);
}

std::multimap<int, ConnectionEvent> ServerEventDispatcher::Wait(int timeout) {
    std::multimap<int, ConnectionEvent> connections;
    bool is_signal_recived = false;
    do {
        int fd_dispathcer_wait_time = CalcWaitTime(&timeout);
        if (fd_dispathcer_wait_time == 0) {
            break;
        }
        std::multimap<int, FdEvent> fd_events;
        try {
            fd_events = this->fd_event_dispatcher_.Wait(fd_dispathcer_wait_time);
        } catch (const SignalDelivered &e) {
            is_signal_recived = true;
            break;
        }
        if (fd_events.size() == 0) {
            break;
        }
        this->MergeDuplicateFd(&fd_events);
        for (std::multimap<int, FdEvent>::iterator it = fd_events.begin(); it != fd_events.end(); it++) {
            const int &fd = it->second.fd_;
            const FdEventType &event = it->second.event_;
            if (event == kFdEventTypeUndefined) {
                // Nothing to do;
            } else if (event == kHaveReadableBuffer) {
                connections.insert(std::make_pair(fd, this->CreateConnectionEvent(fd, kHaveReadableBuffer)));
            } else if (event == kEOF) {
                connections.insert(std::make_pair(fd, this->CreateConnectionEvent(fd, kEOF)));
            } else if (event == kChanged) {
                this->RegisterNewConnection(fd);
            } else if (event == kWriteEnd) {
                if (this->scheduled_close_.find(fd) == this->scheduled_close_.end()) {
                    connections.insert(std::make_pair(fd, this->CreateConnectionEvent(fd, kWriteEnd)));
                }
            } else if (event == kFdEventFail) {
                connections.insert(std::make_pair(fd, this->CreateConnectionEvent(fd, kFdEventFail)));
            }
        }
    } while (connections.size() == 0);
    if (is_signal_recived) {
        connections.insert(std::make_pair(PROCESS_CHENGED_FD, ConnectionEvent(kChildProcessChanged, NON_EXIST_FD, NON_EXIST_FD, NON_EXIST_FD)));
    }
    OverrideTimeoutEvent(&connections);
    return connections;
}

const std::string &ServerEventDispatcher::get_read_buffer(const int &fd) const {
    return this->fd_event_dispatcher_.get_read_buffer(fd);
}

void ServerEventDispatcher::add_writen_buffer(const int &fd, const std::string &src) {
    this->fd_event_dispatcher_.add_writen_buffer(fd, src);
}

void ServerEventDispatcher::erase_read_buffer(const int &fd, const std::string::size_type &front, const std::string::size_type &len) {
    this->fd_event_dispatcher_.erase_read_buffer(fd, front, len);
}

bool ServerEventDispatcher::IsEmptyWritebleBuffer(const int &fd) {
    return this->fd_event_dispatcher_.IsEmptyWritebleBuffer(fd);
}
// ------------------------------------------------------------------------ //
//                                                                          //
//                       Exception class SignalDelivered                    //
//                                                                          //
// ------------------------------------------------------------------------ //

SignalDelivered::SignalDelivered(const int sigid) : sigid_(sigid) {
}

const char* SignalDelivered::what() const throw() {
    // std::stringstream message;
    // message << "SignalDelivered: delivered signal '" << this->sigid_ << "'";
    // return message.str().c_str();
    return "SignalDelivered: delivered signal";
}

const int &SignalDelivered::GetSigid() const {
    return this->sigid_;
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

    for (int i = 0; i < 10; i++) {
        vector<FdEvent> event_content =  event_dispatcher.Wait(1000 * 1000);
        for (size_t j = 0; j < event_content.size(); j++) {
            if (event_dispatcher.IsEmptyWritebleBuffer(event_content.at(j).fd_)) {
                event_dispatcher.Unregister(event_content.at(j).fd_);
            } else {
                cout << "[fd, event, buffer]: [" << event_content.at(j).fd_ << ", " << event_content.at(j).event_ << ", '" << event_dispatcher.get_read_buffer(event_content.at(j).fd_) << "']" << endl;
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
    for (int i = 0; i < 205; i++) {
        fde.add_writen_buffer(STDOUT_FILENO, "0123456789");
    }
    while (!fde.IsEmptyWritebleBuffer(STDOUT_FILENO)) {
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


/*
// func for test RelatedFds class //
int main() {
    RelatedFds fds;

    fds.RegisterSocketFd(100);
    fds.RegisterConnectionFd(110, 100);
    fds.RegisterFileFd(111, 110);
    fds.RegisterFileFd(112, 110);

    fds.RegisterConnectionFd(120, 100);
    fds.RegisterFileFd(121, 120);
    fds.RegisterFileFd(122, 120);

    // fds.RegisterSocketFd(200);
    // fds.RegisterConnectionFd(210, 200);
    // fds.RegisterFileFd(211, 210);
    // fds.RegisterFileFd(212, 210);

    // fds.RegisterConnectionFd(220, 200);
    // fds.RegisterFileFd(221, 220);
    // fds.RegisterFileFd(222, 220);


    fds.print();
    std::cerr << "==========================================" << std::endl;
    fds.UnregisterFileFd(121);
    fds.print();
    std::cerr << "==========================================" << std::endl;
    fds.UnregisterConnectionFd(110);
    fds.print();
    std::cerr << "==========================================" << std::endl;
    fds.UnregisterSocketFd(100);
    fds.print();
}

// */

/*
#include <iostream>
#include <Unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
using namespace std;

void child(const int sv[2]) {
    char *str = "from child";
    write(sv[0], str, 10);
    write(sv[1], str, 10);
    std::cerr << "child  : sv: [" << sv[0] << ", " << sv[1] << "]" << endl;
    system("sleep 1");
}

void pairent(const int sv[2]) {
    usleep(10000);
    std::cerr << "pairent: sv: [" << sv[0] << ", " << sv[1] << "]" << endl;

    FdEventDispatcher dispatcher;
    dispatcher.Register(sv[0], kFile);
    dispatcher.Register(sv[1], kFile);
    for (size_t j = 0; j < 2; j++) {
        vector<FdEvent> event = dispatcher.Wait(-1);

        for (size_t i = 0; i < event.size(); i++) {
            cerr << event[i].fd_ << ": " << dispatcher.get_read_buffer(event[i].fd_) << endl;
        }
    }   
    char *buf[10];
    fcntl(sv[0], F_SETFL, O_NONBLOCK);
    fcntl(sv[1], F_SETFL, O_NONBLOCK);
    cerr << read(sv[0], buf, 1) << endl;
    cerr << read(sv[1], buf, 1) << endl;

}

int main() {
    int sv[2];

    socketpair(AF_UNIX, SOCK_STREAM, 0, sv);

    pid_t cpid = fork();

    if (cpid == 0) {
        child(sv);
    } else {
        pairent(sv);
    }
    return 0;
}
// */


/*
#include <iostream>
using namespace std;
int main() {
    FdEventDispatcher disp;

    disp.Register(STDIN_FILENO, kFile);
    for (size_t i = 0; i < 10; i++) {
        cerr << "waiting" << endl;
        std::multimap<int, FdEvent> event = disp.Wait(1000);
        for (auto it = event.begin(); it != event.end(); it++) {
            cout << it->second.fd_ << ' ' << it->second.event_ << endl;
            // cerr << disp.get_read_buffer(it->second.fd_) << endl;
            if (event.count(it->second.fd_) == 1 && it->second.event_ == kFdEventFail) {
                disp.Unregister(it->second.fd_);
            } 
        }
    }
}
// */
