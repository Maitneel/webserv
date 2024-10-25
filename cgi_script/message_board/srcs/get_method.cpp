#include <fstream>
#include <iostream>
#include <map>
#include <utility>
#include <vector>

#include "defines.hpp"
#include "simple_db.hpp"

#include "string_lib.hpp"
#include "cookie.hpp"

#define debug(s) std::cerr << #s << '\'' << (s) << '\'' << std::endl;

void get_method() {
    // TODO path がやばいのでどうにかする //
    std::ifstream ifs(INDEX_HTML_PATH);
    std::string s;
    std::multimap<std::string, std::string> cookie = parse_cookie();
    SimpleDB auth_db(AUTH_DB_PATH);

    std::cout << "Content-Type:text/html" << "\n";
    std::cout << get_cookie_header(cookie, &auth_db);
    std::cout << "\n";
    while (getline(ifs, s)) {
        std::cout << s << "\n";
    }
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