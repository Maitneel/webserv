#include <stdexcept>
#include <iostream>

#include "HTTPRequest.hpp"
#include "keywords.hpp"

HTTPRequest::HTTPRequest() : method(), request_uri(), protocol() {

}

HTTPRequest::HTTPRequest(const int fd) {
	// TODO
}

HTTPRequest::HTTPRequest(std::string buffer) {
	std::string crlf;
	crlf += CR;
	crlf += LF;
	try {
		size_t front = 0;

		this->method = get_first_token(buffer);
		front += this->method.length();
		if (!is_sp(buffer[front])) {
			std::cerr << "flag1" << std::endl;
			throw InvalidRequest(REQUEST_LINE);
		}
		front++;
		std::string::size_type request_uri_end = buffer.find(' ', front);
		if (request_uri_end == std::string::npos) {
			request_uri_end = buffer.find(crlf, front);
		}
		if (request_uri_end == std::string::npos) {
			std::cerr << "flag2" << std::endl;
			throw InvalidRequest(REQUEST_LINE);
		}
		this->request_uri = buffer.substr(front, request_uri_end - front);
		if (!(is_absolute_uri(this->request_uri) || is_abs_path(this->request_uri))) {
			std::cerr << "flag6" <<std::endl;
			throw InvalidRequest(REQUEST_LINE);
		}
		front = request_uri_end;
		const std::string::size_type crlf_index = buffer.find(crlf, 0);
		if (crlf_index == front) {
			protocol = "HTTP/0.9";
		} else {
			if (!is_sp(buffer.at(front))) {
				std::cerr << "fla5" << std::endl;
				throw InvalidRequest(REQUEST_LINE);
			}
			front++;
			this->protocol = buffer.substr(front, crlf_index - front);
			if (!is_http_version(this->protocol)) {
				std::cerr << this->protocol << std::endl;
				throw InvalidRequest(REQUEST_LINE);
			}
		}
	} catch (const std::out_of_range e) {
		std::cerr << "flag3" << std::endl;
		throw InvalidRequest(REQUEST_LINE);
	}

}

HTTPRequest::HTTPRequest(const HTTPRequest &src) : method(src.method), request_uri(src.request_uri), protocol(src.protocol) {

}

const HTTPRequest &HTTPRequest::operator=(const HTTPRequest &src) {
	if (this == &src) {
		return *this;
	}
	this->method = src.get_method();
	this->request_uri = src.get_request_uri();
	this->protocol = src.get_protocol();
	return *this;
}

HTTPRequest::~HTTPRequest() {

}


// getter 

const std::string &HTTPRequest::get_method() const {
	return this->method;
}

const std::string &HTTPRequest::get_request_uri() const {
	return this->request_uri;
}

const std::string &HTTPRequest::get_protocol() const {
	return this->protocol;
}


// exception class

HTTPRequest::InvalidRequest::InvalidRequest(t_http_request_except_type except_type_src) : except_type(except_type_src) {

}

const char *HTTPRequest::InvalidRequest::what() const throw() {
	switch (this->except_type) {
	case REQUEST_LINE:
		return "HTTPRequest: inccorected request-line";
		break;
	
	default:
		break;
	}
}