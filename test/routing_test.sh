#!/bin/bash

echo "localhost:8080/hoge"
curl localhost:8080/hoge
echo "-----------------------------"
echo "localhost:8080/hoge/dir_2/"
curl localhost:8080/hoge/dir_2/
echo "-----------------------------"
echo "localhost:8080/hoge/dir_3/"
curl localhost:8080/hoge/dir_3/
echo "-----------------------------"
echo "localhost:8080/hoge/dir_5/"
curl localhost:8080/hoge/dir_5/
echo "-----------------------------"
echo "localhost:8080/hoge/dir_6/"
curl localhost:8080/hoge/dir_6/
echo "-----------------------------"
