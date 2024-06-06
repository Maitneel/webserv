CXX         = c++
CXXFLAGS    := -Wall -Wextra -Werror -std=c++98 -MMD -MP

NAME        = webserv

SRCS_DIR    = ./srcs/
OBJS_DIR    = ./objs/
DEPS_DIR    = ${OBJS_DIR}
INCLUDE_DIR = ./include

SRCS        = main.cpp

ifeq ($(MAKECMDGOALS),debug)
	OBJS        = ${SRCS:%.cpp=${OBJS_DIR}debug_%.o}
	DEPS        = ${SRCS:%.cpp=${DEPS_DIR}debug_%.d}
	CXXFLAGS += -g -fsanitize=address
else
	OBJS        = ${SRCS:%.cpp=${OBJS_DIR}%.o}
	DEPS        = ${SRCS:%.cpp=${DEPS_DIR}%.d}
endif

all:  $(NAME)

${OBJS_DIR} :
	@mkdir -p $@

${OBJS_DIR}%.o : ${SRCS_DIR}%.cpp
	${CXX} ${CXXFLAGS} -I ${INCLUDE_DIR} -c -o $@ $<

${OBJS_DIR}debug_%.o : ${SRCS_DIR}%.cpp
	${CXX} ${CXXFLAGS} -I ${INCLUDE_DIR} -c -o $@ $<

$(NAME): ${OBJS_DIR} $(OBJS)
	$(CXX) ${CXXFLAGS} $(OBJS) -o $(NAME)

clean:
	rm -rf $(OBJS_DIR)

fclean: clean
	rm -rf $(NAME)

re: fclean all

lint:
	cpplint ${SRCS}

debug: ${NAME}

-include ${DEPS}

.PHONY: all clean fclean re test lint
