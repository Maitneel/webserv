#ifndef INCLUDE_PROCCESSING_HPP_
#define INCLUDE_PROCCESSING_HPP_

#include <map>
#include <string>

void get_method(const std::multimap<std::string, std::string> &cookie);
void post_method(const std::multimap<std::string, std::string> &cookie);
void delete_method(const std::multimap<std::string, std::string> &cookie);


#endif // INCLUDE_PROCCESSING_HPP_