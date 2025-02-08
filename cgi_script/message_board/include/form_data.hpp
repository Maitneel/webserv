#ifndef INCLUDE_FORM_DATA_HPP_
#define INCLUDE_FORM_DATA_HPP_

#include <string>
#include <map>

class FormDataParameters {
 public:
    std::map<std::string, std::map<std::string, std::string> > parameter_;

    FormDataParameters(const std::string &single_content);
    ~FormDataParameters();
};

// Content-type が multipart/fomr-data じゃない場合exceptionを投げる
class FormDataBody {
 private:
    const std::string buffer_;
    std::map<std::string, std::string> body_; // name, body;
 public:
    std::map<std::string, FormDataParameters> parameters_; // name, parameters;
    FormDataBody(const int &length);
    ~FormDataBody();

    const std::string &get_body(const std::string &name) const;
};

#endif // INCLUDE_FORM_DATA_HPP_