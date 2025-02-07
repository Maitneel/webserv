NAME = message_board.cgi

CXX = c++
CXXFLAGS = -Wall -Wextra

SRCS_DIR = ./srcs
INCLUDE = ./include
OBJS_DIR = ./objs

SRCS = 	main.cpp \
		get_method.cpp \
		post_method.cpp \
		delete_method.cpp \
		gen_html.cpp \
		formated_string.cpp \
		string_lib.cpp \
		cookie.cpp \
		simple_db.cpp \
		form_data.cpp \



RESOURCE_DIR = ./resource

OBJS = ${SRCS:%.cpp=${OBJS_DIR}/%.o}

all : ${NAME}

${NAME} : ${OBJS_DIR} ${OBJS} ${RESOURCE_DIR}
	${CXX} ${CXXFLAGS} -o ${NAME} ${OBJS}

${OBJS_DIR}/%.o : ${SRCS_DIR}/%.cpp
	${CXX} ${CXXFLAGS} -I ${INCLUDE} -I ${DB_DIR}/include -I ${BODY_PARSER_DIR}/include -c -o $@ $<

${OBJS_DIR} :
	mkdir ${OBJS_DIR}

${RESOURCE_DIR} :
	mkdir ./resource
	mkdir ./resource/public
	mkdir ./resource/private

clean :
	rm -rf ${OBJS_DIR}

fclean : clean
	rm -rf ${NAME}
	rm -rf ${TEST_FILE}

re : fclean all

${TEST_FILE} : ${NAME} ${SRCS_DIR}/main.cpp
	${CXX} ${CXXFLAGS} -o ${TEST_FILE} -I ${INCLUDE} ${NAME} ${SRCS_DIR}/main.cpp

test : ${TEST_FILE} 