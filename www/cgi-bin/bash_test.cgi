#!/bin/bash

# example CGI in bash

# html_content:
date=$(date)
html_content="<!DOCTYPE html><html><body><h2>Current time: $date </h2><a href='/'>go home</a></body></html>"
content_length=${#html_content}

# header
echo -n -e "Status: 200 OK\r\n"
echo -n -e "Server: $SERVER_NAME\r\n"
echo -n -e "Content-Type: text/html\r\n"
echo -n -e "Content-Length: $content_length\r\n"

# line that separates header from body
echo -n -e "\r\n"

# body with html content
echo -n -e "$html_content\r\n"

# debug:
# >&2 echo "debug:" $SERVER_NAME
