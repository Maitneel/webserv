#ifndef INCLUDE_BODY_PARSER_HPP_
#define INCLUDE_BODY_PARSER_HPP_

#include <string>
#include <vector>
#include <map>
#include <utility>

class ContentParameters {
 public:
    std::string key_;
    std::map<std::string, std::string> parameter_;

    ContentParameters(const std::string &str);
};

class ContentElement {
 public:
    std::string name_;
    std::string body_;
    std::map<std::string, ContentParameters> parameter_;

    ContentElement(const std::string &str);
    ~ContentElement();
};

class BodyParser {
 private:
    const std::string buffer_;
 public:
    std::map<std::string, ContentElement> parsed_body_;
    BodyParser();
    ~BodyParser();

    const std::string &get_body(const std::string &name);
    const std::string &get_body();
};

#endif // INCLUDE_BODY_PARSER_HPP_
