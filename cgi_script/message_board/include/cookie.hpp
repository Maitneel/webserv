#ifndef INCLUDE_COOKIE_HPP_
#define INCLUDE_COOKIE_HPP_

#include <string>
#include <map>

#include "simple_db.hpp"

std::string get_remove_cookie(std::string name);
std::string gen_new_auth_cookie(SimpleDB *auth_db);
std::string get_cookie_header(const std::multimap<std::string, std::string> &cookie, SimpleDB *auth_db);
std::multimap<std::string, std::string> parse_cookie();

#endif //INCLUDE_COOKIE_HPP_