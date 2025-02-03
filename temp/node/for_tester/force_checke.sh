#!/bin/bash

for i in {200..599}; do
    yes "" | ./tester http://127.0.0.1:8080
done;