#include <iostream>

#include "hoge.hpp"
#include <sys/types.h>
#include <sys/socket.h>
#include <cstring>
#include <netdb.h>
#include <cstdlib>
#include <unistd.h>
#include <sstream>
#include <string>
#include <iostream>
#include <time.h>
#include <algorithm>
#include <iterator>
#include <map>

#include "HTTPRequest.hpp"

#define MAX_BACKLOG 5
#define BUFFER_SIZE 1024


std::string get_formated_date() {
    time_t now = time(NULL);
    struct tm *now_tm = localtime(&now);
    std::string now_string(asctime(now_tm));
    now_string.erase(remove(now_string.begin(), now_string.end(), '\n'), now_string.end());
    return now_string;    
}

int listen_socket(const char *port)
{
    struct addrinfo hints, *res, *ai;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if (getaddrinfo(NULL, port, &hints, &res) != 0)
    {
        // TODO: error handling
        std::exit(1);
    }

    for (ai = res; ai; ai=ai->ai_next) {
        int sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
        if (sock < 0) {
            close(sock);
            continue;
        }
        if (bind(sock, res->ai_addr, res->ai_addrlen) < 0)
        {
            close(sock);
            continue;
        }
        if (listen(sock, MAX_BACKLOG) < 0)
        {
            close(sock);
            continue;
        }
        freeaddrinfo(res);
        return sock;
    }
    return -1;
}

std::string read_request(int fd)
{
    char buf[BUFFER_SIZE];

    int n_read = recv(fd, buf, BUFFER_SIZE-1, 0);
    if (n_read <= 0)
    {
        // TODO: error handling
        std::exit(1);
    }

    return std::string(buf, n_read);
}

void response_to_client(int fd, std::string content)
{
    std::string response;
    std::ostringstream oss;

    oss << content.size() + 2;
	response += "HTTP/1.1 200 OK\r\n";
	response += "Content-Type: text/html\r\n";
	response += "Content-Length: ";
    response += oss.str() + "\r\n";
	response += "\r\n";
	response += content;
	response += "\r\n";

    send(fd, response.c_str(), response.length(), 0);
    return ;
}

void server_main(int server_fd)
{
    while(true)
    {
        struct sockaddr_storage addr;
        socklen_t addrlen = sizeof addr;

        int sock = accept(server_fd, (struct sockaddr*)&addr, &addrlen);
        if (sock < 0){
            // TODO: error handling
            std::exit(1);
        }
        // TODO: non blocking...
        std::string request_content = read_request(sock);
        std::cerr << "resived " << std::endl;
        HTTPRequest request(request_content);
        std::cout << '[' << get_formated_date() << "] " << request.get_method() << ' ' << request.get_request_uri() << ' ' << request.get_protocol() << std::endl;
        std::cout << "    header : {" << std::endl;
        for (std::map<std::string, std::string>::iterator i = request.header.begin(); i != request.header.end(); i++) {
            std::cout << "        " << i->first << ": " << i->second << std::endl;
        }
        std::cout << "    }" << std::endl;
        std::cout << "    body : {" << std::endl;
        std::cout << "        " << request.entity_body << std::endl;
        std::cout << "    }" << std::endl;
        
        response_to_client(sock, request_content);
        close(sock);

        // break させるようの処理 //
        std::string method = request.get_method();
        if (method == "kill" || method == "KILL" || method == "Kill") {
            break;
        }
    }
}


void test_HTTPRequest_class() {
    const char *req_header = "GET / HTTP/1.1\r\nHost: localhost:8080 Connection: keep-alive sec-ch-ua: \"Not/A)Brand\";v=\"8\", \"Chromium\";v=\"126\", \"Google Chrome\";v=\"126\" sec-ch-ua-mobile: ?0 sec-ch-ua-platform: \"macOS\" Upgrade-Insecure-Requests: 1 User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/126.0.0.0 Safari/537.36 Sec-Purpose: prefetch;prerender Purpose: prefetch Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.7 Sec-Fetch-Site: none Sec-Fetch-Mode: navigate Sec-Fetch-User: ?1 Sec-Fetch-Dest: document Accept-Encoding: gzip, deflate, br, zstd Accept-Language: ja,en-US;q=0.9,en;q=0.8";
    HTTPRequest request(req_header);
    std::cout << "method     : " << request.get_method() << std::endl;
    std::cout << "request_uri: " << request.get_request_uri() << std::endl;
    std::cout << "protocol   : " << request.get_protocol() << std::endl;
}

int main() {
    int server_fd;
    const char* port = "8080";

    server_fd = listen_socket(port);
    server_main(server_fd);
    close(server_fd);

    // test_HTTPRequest_class();
    return 0;
}
