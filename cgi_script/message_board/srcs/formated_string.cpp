#include <unistd.h>
#include <string>
#include <sstream>
#include <iomanip>
#include <algorithm>

std::string get_formated_date() {
    struct tm newtime;
    time_t ltime;
    char buf[50];

    ltime = time(&ltime);
    localtime_r(&ltime, &newtime);
    std::string now_string(asctime_r(&newtime, buf));
    now_string.erase(
        remove(now_string.begin(), now_string.end(), '\n'),
        now_string.end());
    return now_string;
}

std::string get_formated_id(const int &n) {
    std::stringstream ss;
    ss << std::setw(4) << std::setfill('0') << n;
    return (std::string(ss.str()));
}
