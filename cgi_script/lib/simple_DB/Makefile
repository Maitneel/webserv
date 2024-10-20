NAME = simple_db
CXX = c++
CXXFLAGS = -Wall -Wextra

SRCS_DIR = ./srcs
INCLUDE = ./include
OBJS_DIR = ./objs

SRCS = 	main.cpp \
		simple_db.cpp \

OBJS = ${SRCS:%.cpp=${OBJS_DIR}/%.o}

all : ${NAME}

${NAME} : ${OBJS_DIR} ${OBJS}
	${CXX} ${CXXFLAGS} -o ${NAME} ${OBJS}

${OBJS_DIR}/%.o : ${SRCS_DIR}/%.cpp
	${CXX} ${CXXFLAGS} -I ${INCLUDE} -c -o $@ $<

${OBJS_DIR} :
	mkdir ${OBJS_DIR}

clean :
	rm -rf ${OBJS_DIR}

fclean : clean
	rm ${NAME}

re : fclean all

echo:
	@echo ${SRCS}
	@echo ${OBJS}
