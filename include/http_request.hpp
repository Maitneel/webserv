#ifndef HTTP_REQUEST_HPP
# define HTTP_REQUEST_HPP

#include <string>
#include <stdexcept>

typedef enum e_http_request_except_type {
	REQUEST_LINE
} t_http_request_except_type;

class HTTP_Request {
private:
	// request-line
	std::string method;
	std::string request_uri;
	std::string protocol;
public:
	HTTP_Request();
	HTTP_Request(const int fd);
	HTTP_Request(std::string buffer);
	HTTP_Request(const HTTP_Request &src);
	const HTTP_Request &operator=(const HTTP_Request &src); 
	~HTTP_Request();

	// getter
	const std::string &get_method() const;
	const std::string &get_request_uri() const;
	const std::string &get_protocol() const;

	// exception class
	class incorrect_request : public std::exception {
	private:
		const t_http_request_except_type except_type;
	public:
		incorrect_request(t_http_request_except_type except_type_src);
		// const char *what() const noexcept;
		const char* what() const throw();
	};
};

#endif