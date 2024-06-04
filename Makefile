CXX       = c++
CXXFLAGS  = -Wall -Wextra -Werror -std=c++98
SRCS      = ./srcs/main.cpp

OBJS = ${SRCS:%.cpp=%.o}
NAME = webserv

.PHONY: all clean fclean re test lint

$(NAME): $(OBJS)
	$(CXX) $(OBJS) -o $(NAME)

all: $(NAME)

clean:
	rm -rf $(OBJS)

fclean: clean
	rm -rf $(NAME)

re: fclean all

lint:
	cpplint ${SRCS}