#ifndef INCLUDE_HTTP_REQUEST_HPP_
#define INCLUDE_HTTP_REQUEST_HPP_

#include <exception>
#include <string>
#include <map>
#include <stdexcept>
#include <vector>
#include <utility>
#include <ostream>

typedef enum HTTPRequestExceptTypeEnum {
    kRequestLine,
    kHTTPHeader
} HTTPRequestExceptType;

typedef enum HTTPHeaderExceptTypeEnum {
    kAllow,
    kContentEncoding,
    kContentLength,
    kContentType,
    kDate,
    kExpries,
    kForm,
    kIfModifiedSince,
    kLastModified,
    kPragma,
    kReferer,
    kUserAgent,
    kCONVERT_FAIL,
} HTTPHeaderExceptType;

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
    void valid_form(const std::string &value);
    void valid_if_modified_since(const std::string &value);
    void valid_last_modified(const std::string &value);
    void valid_pragma(const std::string &value);
    void valid_referer(const std::string &value);
    void valid_user_agent(const std::string &value);

    void valid_date_related_header(const std::string &value, HTTPHeaderExceptType exception_type, std::string *store);

    // ヘッダー種別とvalid関数のpair //
    std::vector<std::pair<std::string, void (HTTPRequest::*)(const std::string &)> > validation_func_pair;

 public:
    std::map<std::string, std::string> header_;  // 一時的に public  //
    std::string entity_body_;  // これstringでいいのか要検討 //
    std::vector<std::string> allow_;
    std::string content_encoding_;
    int content_length_;
    MIMEType content_type_;
    std::string date_;
    std::string expires_;
    std::string form_;
    std::string if_modified_since_;
    std::string last_modified_;
    // std::string location; // こいつ response 用っぽいから request にはいらないよね ? //
    std::vector<std::string> pragma_;
    std::string referer_;
    std::vector<std::string> user_agent_;

    explicit HTTPRequest(const int fd);
    explicit HTTPRequest(std::string buffer);
    ~HTTPRequest();

    // getter
    const std::string &get_method() const;
    const std::string &get_request_uri() const;
    const std::string &get_protocol() const;

    // print funcs
    void print_info();
    void print_info(std::ostream &stream);

    // exception class
    class InvalidRequest :  public  std::exception {
     private:
        const HTTPRequestExceptType except_type;
     public :
        explicit InvalidRequest(HTTPRequestExceptType except_type_src);
        // const char *what() const noexcept;
        const char* what() const throw();
    };

    class InvalidHeader : public std::exception {
     private:
            const HTTPHeaderExceptType except_type;
     public:
            explicit InvalidHeader(HTTPHeaderExceptType except_type_src);
            const char *what() const throw();
    };
};

#endif  // INCLUDE_HTTP_REQUEST_HPP_
