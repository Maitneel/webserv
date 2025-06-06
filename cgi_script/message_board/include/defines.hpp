#ifndef INCLUDE_DEFINES_HPP_
#define INCLUDE_DEFINES_HPP_

#ifndef INDEX_HTML_PATH_CSTR
# define INDEX_HTML_PATH_CSTR "./cgi_script/message_board/resource/public/index.html"
#endif

#ifndef JS_FILE_PATH
# define JS_FILE_PATH "./cgi_script/message_board/resource/public/request.js"
#endif

#define INDEX_HTML_PATH (std::string)(INDEX_HTML_PATH_CSTR)

#ifndef PUBLIC_RESOURCE_ROOT_CSTR
# define PUBLIC_RESOURCE_ROOT_CSTR (std::string)("./cgi_script/message_board/resource/public")
#endif
#define PUBLIC_RESOURCE_ROOT (std::string)(PUBLIC_RESOURCE_ROOT_CSTR)

#ifndef PRIVATE_RESOURCE_ROOT_CSTR
# define PRIVATE_RESOURCE_ROOT_CSTR (std::string)("./cgi_script/message_board/resource/private")
#endif
#define PRIVATE_RESOURCE_ROOT (std::string)(PRIVATE_RESOURCE_ROOT_CSTR) 

#ifndef REQUEST_URL
# define REQUEST_URL "/cgi/message_board/"
#endif

#define MESSAGE_DB_PATH (PRIVATE_RESOURCE_ROOT + "/message_db.simple_db")
#define AUTH_DB_PATH (PRIVATE_RESOURCE_ROOT + "/auth_db.simple_db")

#define DB_MESSAGE_COUNT_ID "MESSAGE_COUNT"
#define DB_TIME_STAMP_ID_PREFIX (std::string)("time_stamp_id_")
#define DB_SENDER_PREFIX (std::string)("posted_by_")
#define DB_MESSAGE_ID_PREFIX (std::string)("message_id_")
#define DB_IMAGE_ID_PREFIX (std::string)("image_id_")
#define DB_IMAGE_CONTENT_TYPE_PREFIX (std::string)("content_type__image_")
#define DB_POSTED_BY_USER_POREFIX (std::string)("posted_message_")
#define DB_IMAGE_PATH_PREFIX (std::string)("image_path_image_")

#define USER_ID_LENGTH 16
#define USER_AUTH_STRING_LENGTH 128

#ifndef RESOURCE_IMAGE_PREFIX_CSTR
# define RESOURCE_IMAGE_PREFIX_CSTR "./docs/images_"
#endif

#define RESOURCE_IMAGE_PREFIX (std::string)(RESOURCE_IMAGE_PREFIX_CSTR)

#ifndef IMAGE_URL_PREFIX_CSTR
# define IMAGE_URL_PREFIX_CSTR "./images_"
#endif
#define IMAGE_URL_PREFIX (std::string)(IMAGE_URL_PREFIX_CSTR)

#define COOKIE_USER_ID_KEY (std::string)("user")
#define COOKIE_USER_AUTH_KEY (std::string)("auth_key")
// とりあえず1日 //
#define COOKIE_MAX_AGE ("86400")
#define COOKIE_PATH REQUEST_URL



// #define RESOURCE_IMAGE_PREFIX (std::string)("./docs/image_")

// #define IMAGE_URL_PREFIX (std::string)("./docs/image_")

#endif // INCLUDE_DEFINES_HPP_