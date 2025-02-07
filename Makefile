CXX		 := g++
CXXFLAGS	:= -Wall -Wextra -pedantic -std=c++98 -MMD -MP -g -O3

NAME		= webserv

SRCS_DIR		= ./srcs/
OBJS_DIR		= ./objs/
HTTP_DIR		= http/
UTILS_DIR		= utils/
SERVER_DIR  	= server/
CONFIG_DIR  	= config/
CGI_DIR 	= cgi/
DEPS_DIR		= ${OBJS_DIR}
INCLUDE_DIR 	= ./include

PHP_PATH		= ${shell which php}
PYTHON3_PATH	= ${shell which python3}

SRCS		=	main.cpp \
				${HTTP_DIR}http_request.cpp \
				${HTTP_DIR}http_response.cpp \
				${HTTP_DIR}http_validation.cpp \
				${HTTP_DIR}getters.cpp \
				${HTTP_DIR}http_header.cpp \
				${HTTP_DIR}chunk_decoder.cpp \
				${HTTP_DIR}chunk_encoder.cpp \
				${HTTP_DIR}http_exception.cpp \
				${SERVER_DIR}poll_selector.cpp \
				${SERVER_DIR}server.cpp \
				${SERVER_DIR}http_context.cpp \
				${SERVER_DIR}event_dispatcher.cpp \
				${SERVER_DIR}generate_autoindex.cpp \
				${CONFIG_DIR}config.cpp \
				${CONFIG_DIR}config_parser.cpp \
				${UTILS_DIR}split.cpp \
				${UTILS_DIR}safe_to_integer.cpp \
				${UTILS_DIR}string.cpp \
				${UTILS_DIR}int_to_string.cpp \
				${UTILS_DIR}file_signatures.cpp \
				${CGI_DIR}cgi_server.cpp \
				${CGI_DIR}set_meta_valiable.cpp \
				${CGI_DIR}valid_words.cpp \
				${CGI_DIR}cgi_response.cpp \


KERNEL	  := ${shell uname -s}
GNU		 := ${shell ls ${shell echo ${PATH} | sed 's/:/ /g'} | grep -e '^g++-' | head -n 1}
CPP_FILES   := ${shell find ${SRCS_DIR} -name "*.cpp"}
HPP_FILES   := ${shell find ${INCLUDE_DIR} -name "*.hpp"}

ifeq (${GNU},g++-13)
	CXX = ${GNU}
endif

ifeq ($(MAKECMDGOALS),debug)
	OBJS		= ${SRCS:%.cpp=${OBJS_DIR}/debug_%.o}
	DEPS		= ${SRCS:%.cpp=${DEPS_DIR}/debug_%.d}
	CXXFLAGS += -g -fsanitize=address
#	ifeq (${KERNEL},Darwin)
#		ifneq (${GNU},)
#			CXX = ${GNU}
#		endif
#	endif
else
	OBJS		= ${SRCS:%.cpp=${OBJS_DIR}/%.o}
	DEPS		= ${SRCS:%.cpp=${DEPS_DIR}/%.d}
endif

ifneq (${PHP_PATH},)
	CXXFLAGS += -D PHP_PATH=\"${PHP_PATH}\"
endif

ifneq (${PYTHON3_PATH},)
	CXXFLAGS += -D PYTHON3_PATH=\"${PYTHON3_PATH}\"
endif

all:  $(NAME)

${OBJS_DIR} :
	mkdir -p $@
	mkdir -p ${OBJS_DIR}${HTTP_DIR}
	mkdir -p ${OBJS_DIR}${UTILS_DIR}
	mkdir -p ${OBJS_DIR}${SERVER_DIR}
	mkdir -p ${OBJS_DIR}${CONFIG_DIR}
	mkdir -p ${OBJS_DIR}${CGI_DIR}

${OBJS_DIR}/%.o : ${SRCS_DIR}/%.cpp
	${CXX} ${CXXFLAGS} -I ${INCLUDE_DIR} -c -o $@ $<

${OBJS_DIR}/debug_%.o : ${SRCS_DIR}/%.cpp
	${CXX} ${CXXFLAGS} -I ${INCLUDE_DIR} -c -o $@ $<

$(NAME): ${OBJS_DIR} $(OBJS)
	$(CXX) ${CXXFLAGS} $(OBJS) -o $(NAME)

clean:
	rm -rf $(OBJS_DIR)

fclean: clean
	rm -rf $(NAME)

re: fclean all

lint:
	cpplint ${CPP_FILES}
	cpplint ${HPP_FILES}

debug: ${NAME}

test: debug
	./test/minimum_run_test.sh ./webserv

run: debug
	./${NAME} ./config/default.conf

r: run

-include ${DEPS}

echo:
	@echo ${OBJS}
	@echo ${HOGE}
	@echo ${CXX13}
	@echo gun: ${GUN}

show_stat:
	uname -a
	${CXX} --version

tester: debug
	./${NAME} ./config/tester.conf

.PHONY: all clean fclean re test lint
