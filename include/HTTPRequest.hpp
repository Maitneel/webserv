#ifndef HTTPRequest_HPP
# define HTTPRequest_HPP

#include <string>
#include <stdexcept>

typedef enum e_HTTPRequest_except_type {
	REQUEST_LINE
} t_http_request_except_type;

class HTTPRequest {
private:
	// internal variable
	bool is_simple_request;

	// request-line
	std::string method;
	std::string request_uri;
	std::string protocol;
public:
	HTTPRequest();
	HTTPRequest(const int fd);
	HTTPRequest(std::string buffer);
	HTTPRequest(const HTTPRequest &src);
	const HTTPRequest &operator=(const HTTPRequest &src); 
	~HTTPRequest();

	// getter
	const std::string &get_method() const;
	const std::string &get_request_uri() const;
	const std::string &get_protocol() const;

	// exception class
	class InvalidRequest : public std::exception {
	private:
		const t_http_request_except_type except_type;
	public:
		InvalidRequest(t_http_request_except_type except_type_src);
		// const char *what() const noexcept;
		const char* what() const throw();
	};
};

#endif