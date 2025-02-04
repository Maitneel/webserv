#ifndef INCLUDE_EVENT_DISPATCHER_HPP_
#define INCLUDE_EVENT_DISPATCHER_HPP_

#include <poll.h>

#include <string>
#include <vector>
#include <map>
#include <set>
#include <exception>

#include "http_request.hpp"
#include "http_response.hpp"


#define TIMEOUT_LENGTH_USEC 1000000000

#define SEC_PER_MS 1000
#define SEC_PER_USEC 1000000

long long get_usec();

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
    kNoBuffer,
    kDidNotRead
} ReadWriteStatType;

#define BUFFER_ERASE_LENGTH 1000000

class FdManager {
 private:
    const int fd_;
    const FdType type_;
    std::string read_buffer_;
    std::string writen_buffer_;
    ReadWriteStatType write_status_;

    std::string::size_type write_head_;


 public:
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
    kEOF,
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
    std::set<int> registerd_read_fds_;
    std::vector<pollfd> poll_fds_;

    std::multimap<int, FdEvent> ReadBuffer();
    std::multimap<int, FdEvent> WriteBuffer();
    std::multimap<int, FdEvent> GetErrorFds();

    static const std::string empty_string_;

    void UpdatePollEvents();
    std::multimap<int, FdEvent> MergeEvents(const std::multimap<int, FdEvent> &read_event, const std::multimap<int, FdEvent> &write_events, const std::multimap<int, FdEvent> &error_events);

 public:
    FdEventDispatcher();
    ~FdEventDispatcher();

    void Register(const int &fd, const FdType &type);
    void Unregister(const int &fd);
    void UnregisterReadEvent(const int &fd);
    std::multimap<int, FdEvent> Wait(int timeout);

    const std::string &get_read_buffer(const int &fd) const;
    void add_writen_buffer(const int &fd, const std::string &src);
    void erase_read_buffer(const int &fd, const std::string::size_type &front, const std::string::size_type &len);
    bool IsEmptyWritebleBuffer(const int &fd);
};

typedef enum ServerEventTypeEnum {
    kUnknownEvent,                 // 0
    kReadableRequest,              // 1
    kRequestEndOfReaded,           // 2
    kReadableFile,                 // 3
    kFileEndOfRead,                // 4
    kresponseWriteEnd,             // 5
    kFileWriteEnd,                 // 6
    kChildProcessChanged,          // 7
    kTimeout,                      // 8
    kServerEventFail               // 9
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

    // 関数が返す用の変数 //
    static const std::set<AnyFdType> empty_any_fd_type_set_;

 public:
    RelatedFds();
    ~RelatedFds();

    // こいつらgetPairentといいつつ、自分自身が当てはまる場合返してるのキモいかも //
    int GetPairentSocket(const AnyFdType &fd);
    int GetPairentConnection(const FileFdType &fd);

    const std::set<AnyFdType> &GetSocketChildren(const SocketFdType &fd);
    const std::set<FileFdType> &GetConnectionChildren(const ConnectionFdType &fd);

    void RegisterSocketFd(const SocketFdType &socket_fd);
    void RegisterConnectionFd(const ConnectionFdType &connection_fd, const SocketFdType &socket_fd);
    void RegisterFileFd(const FileFdType &file_fd, const ConnectionFdType &connection_fd);

    void UnregisterSocketFd(const SocketFdType &socket_fd);
    void UnregisterConnectionFd(const ConnectionFdType &connection_fd);
    void UnregisterFileFd(const FileFdType &file_fd);

    FdType GetType(const AnyFdType &fd) const;
    const std::set<AnyFdType> &GetChildrenFd(const int &fd);

    // この関数auto使ってるのでC++98環境では動かない //
    // void print();  // for debug;
};

#define PROCESS_CHENGED_FD -1
#define NON_EXIST_FD -1

class ServerEventDispatcher {
 private:
    FdEventDispatcher fd_event_dispatcher_;
    RelatedFds registerd_fds_;
    std::set<int> scheduled_close_;
    std::map<int, long long> continue_connection_until_;

    void RegisterNewConnection(const int &socket_fd);
    ConnectionEvent CreateConnectionEvent(const int &fd, const FdEventType &fd_event);
    ConnectionEvent CreateConnectionEvent(const int &fd, const ServerEventType &fd_event);
    void MergeDuplicateFd(std::multimap<int, FdEvent> *events);
    std::set<int> CheckTimeout();
    void OverrideTimeoutEvent(std::multimap<int, ConnectionEvent> *events);
    int CalcWaitTime(int *timeout);

 public:
    ServerEventDispatcher();
    ~ServerEventDispatcher();

    void RegisterSocketFd(const int &socket_fd);
    void RegisterFileFd(const int &file_fd, const int &connection_fd);
    void UnregisterConnectionFd(const int &connection_fd);
    void UnregisterConnectionReadEvent(const int &fd);
    void UnregisterFileFd(const int &file_fd);
    void UnregisterWithClose(const int &fd);

    std::multimap<int, ConnectionEvent> Wait(int timeout);

    const std::string &get_read_buffer(const int &fd) const;
    void add_writen_buffer(const int &fd, const std::string &src);
    void erase_read_buffer(const int &fd, const std::string::size_type &front, const std::string::size_type &len);
    bool IsEmptyWritebleBuffer(const int &fd);
};

class SignalDelivered : std::exception {
 public:
    explicit SignalDelivered(const int sigid);
    const char* what() const throw();

    const int &GetSigid() const;

 private:
    const int sigid_;
};

#endif  // INCLUDE_EVENT_DISPATCHER_HPP_
