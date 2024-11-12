#!/bin/bash

NAME=webserv

find src include -name '*.cpp' -o -name '*.hpp' | entr -d sh -c '
    echo "\033[33mRebuilding and running...\033[0m"
    if make; then
        pkill -9 $NAME
        ./$NAME &
    else
        echo "Build failed."
    fi
'