#!/bin/bash

ls -al /home/workspace
ls -al /home/workspace/webserve
# /home/workspace/webserve/webserv
# /home/workspace/webserve/webserv /home/workspace/webserve/temp.conf
echo ${WEBSERVE_PATH}

exec $@

