#include <fstream>
#include <iostream>
#include <map>
#include <utility>
#include <vector>

#include "defines.hpp"
#include "simple_db.hpp"

#include "gen_html.hpp"
#include "string_lib.hpp"
#include "cookie.hpp"

#define debug(s) std::cerr << #s << '\'' << (s) << '\'' << std::endl;

void get_method(const std::multimap<std::string, std::string> &cookie) {
    // TODO path がやばいのでどうにかする //
    std::ifstream ifs(INDEX_HTML_PATH_CSTR);
    std::string s;
    SimpleDB auth_db(AUTH_DB_PATH);

    if (!ifs) {
        SimpleDB message_db(MESSAGE_DB_PATH);
        create_index_html(message_db);
        ifs.open(INDEX_HTML_PATH_CSTR);
    }

    std::cout << "Content-Type:text/html" << "\n";
    std::cout << get_cookie_header(cookie, &auth_db);
    std::cout << "\n";
    // while (getline(ifs, s)) {
    //     debug(s);
    //     std::cout << s << "\n";
    // }
    std::cout << ifs.rdbuf();
    std::cout << std::endl;
}


// */
/*
using std::string;
int main() {
    string str = "hoge;hoge";
    std::vector<string> s = escaped_quote_split(str, ";");
    for (size_t i = 0; i < s.size(); i++) {
        debug(s[i]);
    }
    
}

// */