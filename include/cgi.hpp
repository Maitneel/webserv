#ifndef INCLUDE_CGI_HPP_
#define INCLUDE_CGI_HPP_

#include <string>
#include "http_request.hpp"

struct CGIInfo {
 public:
    int writing_fd;
    int reading_fd;
    pid_t pid;

    CGIInfo() : writing_fd(-1), reading_fd(-1), pid(0) {}
    CGIInfo(const int &writing_fd_arg, const int &reading_fd_arg,  const pid_t &pid_arg) : writing_fd(writing_fd_arg), reading_fd(reading_fd_arg), pid(pid_arg) {}
    CGIInfo(const CGIInfo & src) : writing_fd(src.writing_fd), reading_fd(src.reading_fd), pid(src.pid) {}
};

CGIInfo call_cgi_script(const HTTPRequest &request, const std::string &cgi_script_path);
char **make_env_array(const HTTPRequest &request);

#endif  // INCLUDE_CGI_HPP_
