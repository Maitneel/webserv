#!/bin/bash

WEBSERV=$1

${WEBSERV} &
until curl localhost:8080 ; do
    sleep 1;
done
kill $!
