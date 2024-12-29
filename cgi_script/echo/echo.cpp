#include <iostream>
#include <unistd.h>
#include <cstring>

int get_content_length(char **env) {
    const std::string content_length_str = "CONTENT_LENGTH";
    for (size_t i = 0; env[i] != NULL; i++) {
        std::string s = env[i];
        std::string::size_type eq_index = s.find("=");
        if (s.substr(0, eq_index) == content_length_str) {
            std::string len = s.substr(eq_index + 1);
            return atoi(len.c_str());
        }
    }
    return 0;
}

int main(int argc, char **argv, char **env) {

    std::cout << "Content-Type:text/html" << "\n"; 
    std::cout << "\n";

    std::cout << "<!DOCTYPE html>" << "\n";
    std::cout << "<html lang=\"en\">" << "\n";
    std::cout << "<head>" << "\n";
    std::cout << "    <title>echo</title>" << "\n";
    std::cout << "</head>" << "\n";
    std::cout << "<body>" << "\n";

    int content_length = get_content_length(env);
    char buffer[content_length];
    bzero(buffer, content_length + 1);
    read(STDIN_FILENO, &buffer, content_length + 1);
    std::cout << "    <h1>Hello! " << buffer << "</h1>" << "\n";
    int i = 0;
    for (i = 0; env[i] != NULL; i++) {
        std::cout << "    " << env[i] << "<br>" << "\n";
    }

    std::cout << "</body>" << "\n";
    std::cout << "</html>" << "\n";
    std::cout << "\n";
    
    return 0;

    (void)(argc);
    (void)(argv);
}