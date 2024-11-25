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
    kReadWriteStatUndefined,
    kSuccess,
    kReturnedZero,
    kFail,
    kContinue,
    kNoBuffer
} ReadWriteStatType;

class FdManager {
 private:
    const int fd_;
    const FdType type_;
    std::string read_buffer_;
    std::string writen_buffer_;
    ReadWriteStatType write_status_;


 public:
    bool is_eof_;

    FdManager(const int &fd, const FdType &type);
    ~FdManager();

    ReadWriteStatType Read();
    ReadWriteStatType Write();
    const std::string &get_read_buffer() const;
    void add_writen_buffer(const std::string &src);
    void erase_read_buffer(const std::string::size_type &front, const std::string::size_type &len);
    bool IsEmptyWritebleBuffer();
    const FdType &get_type() const;
};

typedef enum FdEventTypeEnum {
    kFdEventTypeUndefined,
    kHaveReadableBuffer,
    kChanged,
    kWriteEnd,
    kFdEventFail
} FdEventType;

struct FdEvent {
    int fd_;
    FdEventType event_;

    FdEvent(const int &fd_arg, const FdEventType &event_arg);
};

class FdEventDispatcher {
 private:
    // FdManager をポインタで返す関数があるのでほぼpublic //
    // どこがFdManagerの所有権を持つべきかよくわからない //
    std::map<int, FdManager> fds_;
    std::vector<pollfd> poll_fds_;

    std::vector<FdEvent> ReadBuffer();
    std::vector<FdEvent> WriteBuffer();

    void UpdatePollEvents();
 public:
    FdEventDispatcher();
    ~FdEventDispatcher();

    void Register(const int &fd, const FdType &type);
    void Unregister(const int &fd);
    std::vector<FdEvent> Wait(int timeout);

    const std::string &get_read_buffer(const int &fd) const;
    void add_writen_buffer(const int &fd, const std::string &src);
    void erase_read_buffer(const int &fd, const std::string::size_type &front, const std::string::size_type &len);
    bool IsEmptyWritebleBuffer(const int &fd);
};

// TODO(Maitneel): 命名規則の確認
typedef enum ServerEventTypeEnum {
    kUnknownEvent,
    kReadableRequest,
    kReadableFile,
    kReadableRequestAndFile,
    kResponceWriteEnd_,
    kFileWriteEnd_
} ServerEventType;

struct ConnectionEvent {
    ServerEventType event;
    int socket_fd;
    int connection_fd;
    int file_fd;

    ConnectionEvent();
    ConnectionEvent(
        const ServerEventType &event_arg,
        const int &socket_fd_arg,
        const int &connection_fd_arg,
        const int &file_fd_arg
    );
};

// 変数名の関係性が表現しにくいので型自体に意味を持たせたいという思想 //
typedef int SocketFdType;
typedef int ConnectionFdType;
typedef int FileFdType;
typedef int AnyFdType;

class RelatedFds {
 private:
    std::set<AnyFdType> registerd_fds_;
    std::set<SocketFdType> socket_fds_;
    std::set<ConnectionFdType> connection_fds_;
    std::set<FileFdType> file_fds_;

    // ここpairentはpairentが先なのにchildrenはchildrenがあとなのキモいな... //
    // 英文法わかんないからどっちが正しいのかわかんないけど //
    std::map<AnyFdType, SocketFdType> pairent_socket_;
    std::map<FileFdType, ConnectionFdType> pairent_connection_;
    std::map<SocketFdType, std::set<AnyFdType> > socket_children_;
    std::map<ConnectionFdType, std::set<FileFdType> > connection_childlen_;

    std::map<AnyFdType, FdType> fd_type_;

 public:
    RelatedFds();
    ~RelatedFds();

    int GetPairentSocket(const AnyFdType &fd);
    int GetPairentConnection(const FileFdType &fd);

    void RegistorSocketFd(const SocketFdType &socket_fd);
    void RegistorConnectionFd(const ConnectionFdType &connection_fd, const SocketFdType &socket_fd);
    void RegistorFileFd(const FileFdType &file_fd, const ConnectionFdType &connection_fd, SocketFdType socket_fd = -1);

    void UnregistorSocketFd(const SocketFdType &socket_fd);
    void UnregistorConnectionFd(const ConnectionFdType &connection_fd);
    void UnregistorFileFd(const FileFdType &file_fd);

    FdType GetType(const AnyFdType &fd) const;
    const std::set<AnyFdType> &GetChildrenFd(const int &fd);

    // この関数auto使ってるのでC++98環境では動かない //
    // void print();  // for debug;
};

class ServerEventDispatcher {
 private:
    FdEventDispatcher fd_event_dispatcher_;
    RelatedFds registerd_fds_;

    void RegistorNewConnection(const int &socket_fd);
    ConnectionEvent CreateConnectionEvent(const int &fd, const FdEventType &fd_event);

 public:
    ServerEventDispatcher();
    ~ServerEventDispatcher();

    void RegistorSocketFd(const int &socket_fd);
    void RegistorFileFd(const int &file_fd, const int &connection_fd);
    void UnregistorConnectionFd(const int &connection_fd);
    void UnregistorFileFd(const int &file_fd);
    std::vector<std::pair<int, ConnectionEvent> > Wait(int timeout);

    const std::string &get_read_buffer(const int &fd) const;
    void add_writen_buffer(const int &fd, const std::string &src);
    void erase_read_buffer(const int &fd, const std::string::size_type &front, const std::string::size_type &len);
    bool IsEmptyWritebleBuffer(const int &fd);
};

#endif  // INCLUDE_EVENT_DISPATCHER_HPP_
