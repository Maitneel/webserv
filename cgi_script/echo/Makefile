NAME = echo.cgi

GXX = c++
CXXFLAGS = -Wall -Wextra -Werror -std=c++98

SRCS = echo.cpp
OBJS = ${SRCS:%.cpp=%.o}

all : ${NAME}

${NAME}: ${OBJS}
	${CXX} ${CXXFLAGS} -o $@ ${OBJS}

%.o : %.cpp
	${CXX} ${CXXFLAGS} -c -o $@ $<

clean :
	rm ${OBJS}

fclean : clean
	rm ${NAME}

re : fclean all

echo :
	@echo ${SRCS}