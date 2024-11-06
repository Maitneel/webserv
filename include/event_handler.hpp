#ifndef INCLUDE_EVENT_HANDLER_HPP_
#define INCLUDE_EVENT_HANDLER_HPP_

#include <poll.h>

#include <string>
#include <vector>
#include <map>
#include <set>

#include "http_request.hpp"
#include "http_response.hpp"

typedef enum FdTypeEnum {
    kSocket,
    kConnection,
    kFile
} FdType;

typedef enum ReadWriteStatEnum {
    kSuccess,
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
    FdManager(const int &fd, const FdType &type);
    ~FdManager();

    ReadWriteStatType Read();
    ReadWriteStatType Write();
    const std::string &get_read_buffer();
    void add_writen_buffer(const std::string &src);
    void erase_read_buffer(std::string::size_type &front, std::string::size_type &len);
    const FdType &get_type() const;
};

class FdEventHandler {
 private:
    std::map<int, FdManager *> fds_;
    std::set<int> socket_fds_;
    std::vector<pollfd> poll_fds_;

    std::vector<int> ReadBuffer();

 public:
    FdEventHandler();
    ~FdEventHandler();

    void RegisterSocket(const int &fd);
    void Register(const int &fd, FdManager *fd_manager);
    void Unregister(const int &fd);
  
    std::vector<int> Wait(int timeout);
};

typedef enum FdEventTypeEnum {
    kRequest,
    kResponce,
} FdEventType;

class ServerEventHandler {
 private:
    FdEventHandler FdEvent;
    std::map<int, std::set<int> > related_fd;
 public:
    ServerEventHandler();
    ~ServerEventHandler();

    void RegistorSocketFd();
    std::vector<std::pair<int, FdEventType> > Wait(int timeout); // 戻り値の型は変えたほうが良いかも //
};

#endif // INCLUDE_EVENT_HANDLER_HPP_
