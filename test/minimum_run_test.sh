#!/bin/bash

WEBSERV=$1

${WEBSERV} &
until curl localhost:8080 ; do
    sleep 1;
done

curl -X POST -d "this is body" localhost:8080

kill $!
