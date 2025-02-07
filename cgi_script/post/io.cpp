#include <unistd.h>
#include <cstdlib>
#include <sys/fcntl.h>
#include <string>
#include <iostream>
#include <stdexcept>

#define BUFFER_SIZE 65536

std::string read_data() {
    char *env_ptr = getenv("CONTENT_LENGTH");
    if (env_ptr == NULL) {
        throw std::runtime_error("env not found");
    }
    long long content_length = atoll(env_ptr);
    std::string body;
    while ((long long)(body.length()) < content_length) {
        char buffer[BUFFER_SIZE];
        int read_ret = read(STDIN_FILENO, buffer, BUFFER_SIZE);
        if (read_ret < 0) {
            throw std::runtime_error("write error");
        }
        body += std::string(buffer, read_ret);
    }
    return body;
}

int write_file(const std::string &file_name, std::string *data) {
    std::string file_path = "./docs/";
    file_path += file_name;
    const int fd = open(file_path.c_str(), O_WRONLY | O_CREAT, 0644);
    if (fd < 0) {
        return -1;
    }
    while (data->length()) {
        int to_write_size;
        if ((size_t)(INT_MAX) < data->length()) {
            to_write_size = INT_MAX;
        } else {
            to_write_size = data->length();
        }
        const int write_ret = write(fd, data->c_str(), to_write_size);
        if (write_ret < 0) {
            perror("write");
            close(fd);
            return -1;
        }
        data->erase(0, write_ret);
    }
    close(fd);
    return 0;
}

void write_cgi_responce(const std::string &filename) {
    std::cout << "Content-Type: text/plain\n";
    std::cout << "Status: 201 Created\n";
    std::cout << "Location: /docs/" << filename <<  "\n";
    std::cout << "\n";
    std::cout << "Created\n";

}

void write_error_cgi_response() {
    std::cout << "Content-Type: text/plain\n";
    std::cout << "Status: 403 Forbidden\n";
    std::cout << "\n";
    std::cout << "Forbidden\n";
}