#!/usr/bin/python3
from os import environ
from datetime import datetime
import sys

now = datetime.now()
now_str = now.strftime('%H:%M:%S')

longSection =  ""
for _ in range(39):
    longSection += "<p>Twenty chars!</p>"

longSection += "<p>extra.....</p>"
html_content = (
    "<!DOCTYPE html>\n"
    "<html><body>\n"
    "<a href='/'>go home</a>\n"
    f"{longSection}\n"
    "<a href='/'>go home</a>\n"
    "</body></html>\n"
)

# print('Python debug line', file=sys.stderr)
# for name, value in environ.items():
#     print("{0}: {1}".format(name, value), file=sys.stderr)

print("Status: 200 OK", end='\r\n')
print("Content-Type: text/html", end='\r\n')
print(f"Content-Length: {len(html_content)}", end='\r\n')
if ('SERVER_NAME' in environ.keys()):
    print(f"Server: {environ['SERVER_NAME']}", end='\r\n')
print("", end='\r\n')  # mandatory empty line
print(html_content)
