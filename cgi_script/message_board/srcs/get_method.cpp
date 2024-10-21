#include <fstream>
#include <iostream>

#include "defines.hpp"

void get_method() {
    // TODO path がやばいのでどうにかする //
    std::ifstream ifs(INDEX_HTML_PATH);
    std::string s;
    std::cout << "Content-Type:text/html" << "\n";
    while (getline(ifs, s)) {
        std::cout << s << "\n";
    }
    std::cout << std::endl;
}