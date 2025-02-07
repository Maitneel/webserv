#include <unistd.h>
#include <sys/time.h>
#include <string>
#include <cstdlib>
#include <stdexcept>

#define SEC_PER_USEC 1000000

static long long get_usec() {
    struct timeval tv;
    if (gettimeofday(&tv, NULL)) {
        return -1;
    }
    return (tv.tv_sec * SEC_PER_USEC + tv.tv_usec);
}

std::string read_data();
int write_file(const std::string &file_name, std::string *data);
void write_cgi_responce(const std::string &filename);
void write_error_cgi_response();
void write_200_cgi_response();

std::string to_string(long long n) {
    std::string str;
    if (n == 0) return "0";
    while (n) {
        str += '0' + n % 10;
        n /= 10;
    }
    return str;
}

std::string gen_filename() {
    return to_string(get_usec());
}

void delete_file() {
    char *path_info = getenv("PATH_INFO");
    if (path_info == NULL) {
        throw std::runtime_error("getenv: not found env");
    }
    std::string path = "./docs/";
    path += path_info;
    if (path.at(path.length() - 1) == '/') {
        path.erase(path.length() - 1);
    }
    if (unlink(path.c_str())) {
        throw std::runtime_error("delete fail");
    }
    write_200_cgi_response();
}


int main() {
    char *method = getenv("REQUEST_METHOD");
    std::string method_str(method);

    if (method == NULL) {
        write_error_cgi_response();
        return 0;
    }

    if (method_str == "POST") {
        std::string data = read_data();
        std::string filename = gen_filename();
        if (!write_file(filename, &data)) {
            write_cgi_responce(filename);
        } else {
            write_error_cgi_response();
        }
    } else if (method_str == "DELETE") {
        try {
            delete_file();
        } catch (std::exception &e) {
            write_error_cgi_response();
        }
    } else {
        write_error_cgi_response();
    }

}