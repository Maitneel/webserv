#include <iostream>
#include <fstream>
#include "form_data.hpp"

#define debug(s) std::cerr << #s << '\'' << (s) << '\'' << std::endl;

int main() {
    FormDataBody body;

    ofstream ofs("replaced");
    ofs << body.buffer_ << endl;
    try {
        debug(body.get_body("input_text"));
        debug(body.get_body("input_file"));
    } catch (...) {
        std::cerr << "catch" << std::endl;
    }
    try {
        debug(body.get_body("message"));
        debug(body.get_body("attachment"));
    } catch (...) {
        std::cerr << "catch" << std::endl;
    }

}
