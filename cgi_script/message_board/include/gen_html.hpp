#ifndef INCLUDE_GEN_HTML_HPP_
#define INCLUDE_GEN_HTML_HPP_

#include <string>

#include "simple_db.hpp"

std::string create_template_front();
std::string create_message_div(const SimpleDB &message_db, const std::string &id);
std::string create_template_end();
void create_index_html(SimpleDB &message_db);
void init_index_html();

#endif // INCLUDE_GEN_HTML_HPP_