#ifndef INCLUDE_HTTP_EXCEPTION_HPP_
#define INCLUDE_HTTP_EXCEPTION_HPP_

#include <exception>

class MustToReturnStatus : std::exception {
 public:
    explicit MustToReturnStatus(const int &status_code);
    const char *what() const throw();

    const int &GetStatusCode() const throw();
 private:
    const int status_code_;
};

#endif  // INCLUDE_HTTP_EXCEPTION_HPP_
