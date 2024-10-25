#ifndef INCLUDE_COOKIE_HPP_
#define INCLUDE_COOKIE_HPP_

#include <string>
#include <map>

#include "simple_db.hpp"

std::string get_remove_cookie(std::string name);
std::string gen_user_id(const SimpleDB *auth_db);
std::string gen_auth_str();
void register_auth_info_to_db(SimpleDB *auth_db, std::string user_id, std::string auth_str);
std::string get_auth_cookie_header(const std::string &user_id, const std::string &auth_str);
std::string gen_new_auth_cookie(SimpleDB *auth_db);
bool is_must_update(const SimpleDB &auth_db, const std::multimap<std::string, std::string> &cookie);
bool is_remove_prev_cookie(const SimpleDB &auth_db, const std::multimap<std::string, std::string> &cookie);
std::string gen_new_auth_cookie(SimpleDB *auth_db);
std::string get_cookie_header(const std::multimap<std::string, std::string> &cookie, SimpleDB *auth_db);
std::multimap<std::string, std::string> parse_cookie();

#endif //INCLUDE_COOKIE_HPP_