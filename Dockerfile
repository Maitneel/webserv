FROM ubuntu:24.04

# initialize for container system
RUN apt update && apt install -y g++ make curl git

ENV WEBSERV_PATH=/home/workspace/webserv

# createing webserv binary
COPY ./srcs/ ${WEBSERV_PATH}/srcs/
COPY ./include/ ${WEBSERV_PATH}/include/
COPY ./Makefile/ ${WEBSERV_PATH}/Makefile

RUN make -C ${WEBSERV_PATH} re

# import html files
COPY ./docker_resource/html/ /var/www/html/
RUN git clone https://github.com/Maitneel/chess-timer.git /var/www/html/chess-timer

# cgi program
COPY ./docker_resource/server.conf ${WEBSERV_PATH}/server.conf
COPY ./cgi_script/message_board/ /home/workspace/message_board
RUN IN_DOCKER=true make -C /home/workspace/message_board re


CMD [ "/home/workspace/webserv/webserv", "/home/workspace/webserv/server.conf" ]
