#include <iostream>
#include <stdexcept>

#include <proccessing.hpp>
using namespace std;

int main() {
    
    std::string method = getenv("REQUEST_METHOD");
    std::cerr << method << std::endl;
    try {
        if (method == "GET") {
            get_method();
        } else if (method == "POST") {
            post_method();
        } else if (method == "DELETE") {
            delete_method();
        }
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
        return -1;
    }
    return 0;
}