NAME = simple_db
CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++98

TEST_FILE = a.out

AR = ar

SRCS_DIR = ./srcs
INCLUDE = ./include
OBJS_DIR = ./objs

SRCS = simple_db.cpp \

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
	${CXX} ${CXXFLAGS} -o ${TEST_FILE} -I ${INCLUDE} ${NAME} ${SRCS_DIR}/main.cpp

test : ${TEST_FILE} 