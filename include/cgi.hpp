#ifndef INCLUDE_CGI_HPP_
#define INCLUDE_CGI_HPP_

#include <string>
#include "http_request.hpp"

std::string call_cgi_script(const HTTPRequest &request, const std::string &cgi_script_path);

#endif  // INCLUDE_CGI_HPP_
