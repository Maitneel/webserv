#include <iostream>
#include "body_parser.hpp"
using namespace std;

#define debug(s) std::cerr << #s << '\'' << (s) << '\'' << std::endl;

int main() {
    BodyParser body;

    int count = 0;

    debug(body.parsed_body_.size())
    for (auto it = body.parsed_body_.begin(); it != body.parsed_body_.end(); it++) {
        debug(it->first);
        debug(it->second.body_);
    }


}
