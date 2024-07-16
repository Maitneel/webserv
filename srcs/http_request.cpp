#include <stdexcept>
#include <iostream>

#include "http_request.hpp"
#include "keywords.hpp"

HTTP_Request::HTTP_Request() : method(), request_uri(), protocol() {

}

HTTP_Request::HTTP_Request(const int fd) {
	// TODO
}

HTTP_Request::HTTP_Request(std::string buffer) {
	std::string crlf;
	crlf += CR;
	crlf += LF;
	try {
		size_t front = 0;

		this->method = get_first_token(buffer);
		front += this->method.length();
		if (!is_sp(buffer[front])) {
			std::cerr << "flag1" << std::endl;
			throw incorrect_request(REQUEST_LINE);
		}
		front++;
		std::string::size_type request_uri_end = buffer.find(' ', front);
		if (request_uri_end == std::string::npos) {
			request_uri_end = buffer.find(crlf, front);
		}
		if (request_uri_end == std::string::npos) {
			std::cerr << "flag2" << std::endl;
			throw incorrect_request(REQUEST_LINE);
		}
		this->request_uri = buffer.substr(front, request_uri_end - front);
		if (!(is_absolute_uri(this->request_uri) || is_abs_path(this->request_uri))) {
			std::cerr << "flag6" <<std::endl;
			throw incorrect_request(REQUEST_LINE);
		}
		front = request_uri_end;
		const std::string::size_type crlf_index = buffer.find(crlf, 0);
		if (crlf_index == front) {
			protocol = "HTTP/0.9";
		} else {
			if (!is_sp(buffer.at(front))) {
				std::cerr << "fla5" << std::endl;
				throw incorrect_request(REQUEST_LINE);
			}
			front++;
			this->protocol = buffer.substr(front, crlf_index - front);
			if (!is_http_version(this->protocol)) {
				std::cerr << this->protocol << std::endl;
				throw incorrect_request(REQUEST_LINE);
			}
		}
	} catch (const std::out_of_range e) {
		std::cerr << "flag3" << std::endl;
		throw incorrect_request(REQUEST_LINE);
	}

}

HTTP_Request::HTTP_Request(const HTTP_Request &src) : method(src.method), request_uri(src.request_uri), protocol(src.protocol) {

}

const HTTP_Request &HTTP_Request::operator=(const HTTP_Request &src) {
	if (this == &src) {
		return *this;
	}
	this->method = src.get_method();
	this->request_uri = src.get_request_uri();
	this->protocol = src.get_protocol();
	return *this;
}

HTTP_Request::~HTTP_Request() {

}


// getter 

const std::string &HTTP_Request::get_method() const {
	return this->method;
}

const std::string &HTTP_Request::get_request_uri() const {
	return this->request_uri;
}

const std::string &HTTP_Request::get_protocol() const {
	return this->protocol;
}


// exception class

HTTP_Request::incorrect_request::incorrect_request(t_http_request_except_type except_type_src) : except_type(except_type_src) {

}

const char *HTTP_Request::incorrect_request::what() const throw() {
	switch (this->except_type) {
	case REQUEST_LINE:
		return "HTTP_Request: inccorected request-line";
		break;
	
	default:
		break;
	}
}