#ifndef INCLUDE_EVENT_DISPATCHER_HPP_
#define INCLUDE_EVENT_DISPATCHER_HPP_

#include <poll.h>

#include <string>
#include <vector>
#include <map>
#include <set>

#include "http_request.hpp"
#include "http_response.hpp"

typedef enum FdTypeEnum {
    kUnknownFd,
    kSocket,
    kConnection,
    kFile
} FdType;

typedef enum ReadWriteStatEnum {
    kSuccess,
    kReturnedZero,
    kFail,
    kContinue
} ReadWriteStatType;

class FdManager {
 private:
    const int fd_;
    const FdType type_;
    std::string read_buffer_;
    std::string writen_buffer_;

 public:
    bool is_eof_;

    FdManager(const int &fd, const FdType &type);
    ~FdManager();

    ReadWriteStatType Read();
    ReadWriteStatType Write();
    const std::string &get_read_buffer();
    void add_writen_buffer(const std::string &src);
    void erase_read_buffer(const std::string::size_type &front, const std::string::size_type &len);
    bool HaveWriteableBuffer();
    const FdType &get_type() const;
};

class FdEventDispatcher {
 private:
    // FdManager をポインタで返す関数があるのでほぼpublic //
    // どこがFdManagerの所有権を持つべきかよくわからない //
    std::map<int, FdManager> fds_;
    std::vector<pollfd> poll_fds_;

    std::vector<std::pair<int, FdManager *> >  ReadBuffer();
    ReadWriteStatType WriteBuffer();

    void UpdatePollEvents();
 public:
    FdEventDispatcher();
    ~FdEventDispatcher();

    void Register(const int &fd, const FdType &type);
    void Unregister(const int &fd);
    std::vector<std::pair<int, FdManager *> > Wait(int timeout);

    FdManager *GetBuffer(const int &fd);
};

typedef enum FdEventTypeEnum {
    kUnknownEvent,
    kReadableRequest,
    kReadableFile,
    kReadableRequestAndFile
} FdEventType;

struct ConnectionEvent {
    FdEventType event;
    FdManager *content;
    int socket_fd;
    int connection_fd;
    int file_fd;

    ConnectionEvent();
    ConnectionEvent(
        const FdEventType &event_arg,
        FdManager *content_arg,
        const int &socket_fd_arg,
        const int &connection_fd_arg,
        const int &file_fd_arg
    );
};

class ServerEventDispatcher {
 private:
    FdEventDispatcher fd_event_dispatcher_;
    std::map<int, std::set<int> > related_fd_;
    std::set<int> socket_fds_;
    std::set<int> connection_fds_;
    std::map<int, int> pairent_;

    int GetSocketFd(const int &fd);
    int GetConnectionFd(const int &fd);
    void Unregistor(const int &fd);
    void RegistorNewConnection(const int &socket_fd);
    ConnectionEvent CreateConnectionEvent(const int &fd, FdManager *fd_buffer);

 public:
    ServerEventDispatcher();
    ~ServerEventDispatcher();

    void RegistorSocketFd(const int &fd);
    void RegistorFileFd(const int &fd, const int &connection_fd);
    void UnregistorConnectionFd(const int &fd);
    void UnregistorFileFd(const int &fd);
    void UnRegistorConnectionOrFile(const int &fd);
    std::vector<std::pair<int, ConnectionEvent> > Wait(int timeout);  // 戻り値の型は変えたほうが良いかも //

    FdManager *GetBuffer(const int &fd);
};

#endif  // INCLUDE_EVENT_DISPATCHER_HPP_
