#include <unistd.h>
#include <sys/time.h>
#include <string>

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

std::string gen_filename() {
    return std::to_string(get_usec());
}

int main() {
    std::string data = read_data();
    std::string filename = gen_filename();
    if (!write_file(filename, &data)) {
        write_cgi_responce(filename);
    } else {
        write_error_cgi_response();
    }
}