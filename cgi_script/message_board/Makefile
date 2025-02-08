NAME = message_board.cgi

CXX = c++
CXXFLAGS = -Wall -Wextra -std=c++98

SRCS_DIR = ./srcs
INCLUDE = ./include
OBJS_DIR = ./objs

SRCS =	main.cpp \
		get_method.cpp \
		post_method.cpp \
		body_parser.cpp \
		delete_method.cpp \
		formated_string.cpp \
		cookie.cpp \
		simple_db.cpp \
		form_data.cpp \
		string_lib.cpp \
		gen_html.cpp \


RESOURCE_DIR = ./resource

OBJS = ${SRCS:%.cpp=${OBJS_DIR}/%.o}

# DB_DIR = ../lib/simple_DB
# DATABASE = ${DB_DIR}/simple_db

# BODY_PARSER_DIR = ../lib/body_parser
# BODY_PARSER = ${BODY_PARSER_DIR}/body_parser

all : ${NAME}

${NAME} : ${OBJS_DIR} ${OBJS} ${RESOURCE_DIR} 
	${CXX} ${CXXFLAGS} -o ${NAME} ${OBJS}

${DATABASE} : ${DB_DIR}/srcs/*.cpp
	make -C ${DB_DIR}

${BODY_PARSER} : ${BODY_PARSER_DIR}/srcs/*.cpp
	make -C ${BODY_PARSER_DIR}

${OBJS_DIR}/%.o : ${SRCS_DIR}/%.cpp
	${CXX} ${CXXFLAGS} -I ${INCLUDE} -I ${DB_DIR}/include -I ${BODY_PARSER_DIR}/include -c -o $@ $<

${OBJS_DIR} :
	mkdir ${OBJS_DIR}

${RESOURCE_DIR} : ${OBJS}
	mkdir -p ./resource
	mkdir -p ./resource/public
	mkdir -p ./resource/private

clean :
	rm -rf ${OBJS_DIR}

fclean : clean
	rm -rf ${NAME}
	rm -rf ${TEST_FILE}

re : fclean all

${TEST_FILE} : ${NAME} ${SRCS_DIR}/main.cpp
	${CXX} ${CXXFLAGS} -o ${TEST_FILE} -I ${INCLUDE} ${NAME} ${SRCS_DIR}/main.cpp

test : ${TEST_FILE} 