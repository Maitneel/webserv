#ifndef INCLUDE_CGI_HPP_
#define INCLUDE_CGI_HPP_

#include <string>
#include "http_request.hpp"

struct CGIInfo {
 public:
    int fd;
    pid_t pid;
    bool is_proccess_end;

    CGIInfo() :fd(-1), pid(0), is_proccess_end(false) {}
    CGIInfo(const int &fd_arg,  const pid_t &pid_arg) : fd(fd_arg), pid(pid_arg), is_proccess_end(false) {}
    CGIInfo(const CGIInfo & src) : fd(src.fd), pid(src.pid) {}
    const CGIInfo &operator=(const CGIInfo &rhs) {
        if (this == &rhs) {
            return *this;
        }
        this->fd = rhs.fd;
        this->pid = rhs.pid;
        this->is_proccess_end = rhs.is_proccess_end;
        return *this;
    }
};

CGIInfo call_cgi_script(const HTTPRequest &request, const std::string &cgi_script_path, const std::string &path_info);
char **make_env_array(const HTTPRequest &request, const std::string &path_info);

#endif  // INCLUDE_CGI_HPP_
