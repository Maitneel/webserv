#!/bin/bash

REQUEST_FILE_NAME=request.txt

THIS_FILE_NAME=`echo $0 | sed 's/^.*\///g'`
REQUEST_FILE_PATH=`echo $0 | sed "s/${THIS_FILE_NAME}$/${REQUEST_FILE_NAME}/g"`

echo "request ------------------------"
cat ${REQUEST_FILE_PATH}
echo "response -----------------------"
cat ${REQUEST_FILE_PATH} - | nc localhost 8080