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
};

CGIInfo call_cgi_script(const HTTPRequest &request, const std::string &cgi_script_path);
char **make_env_array(const HTTPRequest &request);

#endif  // INCLUDE_CGI_HPP_
