#ifndef INCLUDE_HTTP_REQUEST_HPP_
#define INCLUDE_HTTP_REQUEST_HPP_

#include <exception>
#include <string>
#include <map>
#include <stdexcept>
#include <vector>
#include <utility>

typedef enum e_HTTPRequest_except_type {
    REQUEST_LINE,
    HTTP_HEADER
} t_http_request_except_type;

typedef enum e_HTTPHeader_except_type {
    ALLOW,
    CONTENT_ENCODING,
    CONTENT_LENGTH,
    CONTENT_TYPE,
    DATE,
    EXPIRES,
    CONVERT_FAIL,
} t_http_header_except_type;

class MIMEType {
 public:
    std::string type;
    std::string subtype;
    std::multimap<std::string, std::string> parameter;
};

class HTTPRequest {
 private:
    // internal variable
    bool is_simple_request;

    // request-line
    std::string method;
    std::string request_uri;
    std::string protocol;

    // request header

    // request header validation function
    void valid_allow(const std::string &value);
    void valid_authorization(const std::string &value);
    void valid_content_encoding(const std::string &value);
    void valid_content_length(const std::string &value);
    void valid_content_type(const std::string &value);
    void valid_date(const std::string &value);
    void valid_expires(const std::string &value);

    // ヘッダー種別とvalid関数のpair //
    std::vector<std::pair<std::string, void (HTTPRequest::*)(const std::string &)> > validation_func_pair;

 public:
    std::map<std::string, std::string> header;  // 一時的に public  //
    std::string entity_body;  // これstringでいいのか要検討 //
    std::vector<std::string> allow;
    std::string content_encoding;
    int content_length;
    MIMEType content_type;
    std::string date;
    std::string expires;

    explicit HTTPRequest(const int fd);
    explicit HTTPRequest(std::string buffer);
    ~HTTPRequest();

    // getter
    const std::string &get_method() const;
    const std::string &get_request_uri() const;
    const std::string &get_protocol() const;

    // exception class
    class InvalidRequest :  public  std::exception {
     private:
        const t_http_request_except_type except_type;
     public :
        explicit InvalidRequest(t_http_request_except_type except_type_src);
        // const char *what() const noexcept;
        const char* what() const throw();
    };

    class InvalidHeader : public std::exception {
     private:
            const t_http_header_except_type except_type;
     public:
            explicit InvalidHeader(t_http_header_except_type except_type_src);
            const char *what() const throw();
    };
};

#endif  // INCLUDE_HTTP_REQUEST_HPP_
