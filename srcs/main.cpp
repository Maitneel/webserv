#include <iostream>
#include "server.hpp"
#include "config.hpp"
#include "http_request.hpp"

void test_HTTPRequest_class() {
    const char *req_header = "GET / HTTP/1.1\r\nHost: localhost:8080 Connection: keep-alive sec-ch-ua: \"Not/A)Brand\";v=\"8\", \"Chromium\";v=\"126\", \"Google Chrome\";v=\"126\" sec-ch-ua-mobile: ?0 sec-ch-ua-platform: \"macOS\" Upgrade-Insecure-Requests: 1 User-Agent: Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/126.0.0.0 Safari/537.36 Sec-Purpose: prefetch;prerender Purpose: prefetch Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/avif,image/webp,image/apng,*/*;q=0.8,application/signed-exchange;v=b3;q=0.7 Sec-Fetch-Site: none Sec-Fetch-Mode: navigate Sec-Fetch-User: ?1 Sec-Fetch-Dest: document Accept-Encoding: gzip, deflate, br, zstd Accept-Language: ja,en-US;q=0.9,en;q=0.8";
    HTTPRequest request(req_header);
    std::cout << "method     : " << request.get_method() << std::endl;
    std::cout << "request_uri: " << request.get_request_uri() << std::endl;
    std::cout << "protocol   : " << request.get_protocol() << std::endl;
}

int main() {
    std::vector<ServerConfig> server_confs = parse_config("");

    Server server(server_confs);

    server.eventLoop();

    test_HTTPRequest_class();
    return 0;
}
