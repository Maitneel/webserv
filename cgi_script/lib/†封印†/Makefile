NAME = body_parser
CXX = c++
CXXFLAGS = -Wall -Wextra

TEST_FILE = a.out

AR = ar

SRCS_DIR = ./srcs
INCLUDE = ./include
OBJS_DIR = ./objs

SRCS = body_parser.cpp \

OBJS = ${SRCS:%.cpp=${OBJS_DIR}/%.o}

all : ${NAME}

${NAME} : ${OBJS_DIR} ${OBJS}
	${AR} -r ${NAME} ${OBJS}

${OBJS_DIR}/%.o : ${SRCS_DIR}/%.cpp
	${CXX} ${CXXFLAGS} -I ${INCLUDE} -c -o $@ $<

${OBJS_DIR} :
	mkdir ${OBJS_DIR}

clean :
	rm -rf ${OBJS_DIR}

fclean : clean
	rm -rf ${NAME}
	rm -rf ${TEST_FILE}

re : fclean all

${TEST_FILE} : ${NAME} ${SRCS_DIR}/main.cpp
	${CXX} ${CXXFLAGS} -std=c++2a -o ${TEST_FILE} -I ${INCLUDE} ${NAME} ${SRCS_DIR}/main.cpp

test : ${TEST_FILE} 
	env -i CONTENT_TYPE="multipart/form-data; boundary=----WebKitFormBoundaryzt8BctbKEjhBGCUB ; hogehoge=fugafuga" ./${TEST_FILE}  < body.txt
