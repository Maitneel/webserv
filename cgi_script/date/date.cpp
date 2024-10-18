#include <iostream>
#include <time.h>

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


int main() {
    std::cout << "Content-Type:text/html" << "\n"; 

    std::cout << "<!DOCTYPE html>" << "\n";
    std::cout << "<html lang=\"en\">" << "\n";
    std::cout << "<head>" << "\n";
    std::cout << "    <title>Date</title>" << "\n";
    std::cout << "</head>" << "\n";
    std::cout << "<body>" << "\n";
    std::cout << "    hello CGI!!<br>" << "\n";
    std::cout << "    generated at " << get_formated_date() << "\n";
    std::cout << "</body>" << "\n";
    std::cout << "</html>" << "\n";
    std::cout << "\n";
}