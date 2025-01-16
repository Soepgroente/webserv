#!/usr/bin/python3
import cgi, os
from os import environ
from datetime import datetime
import cgitb; cgitb.enable()
import sys

now = datetime.now()
now_str = now.strftime('%H:%M:%S')
addLocation = False
form = cgi.FieldStorage()

message = ""
fileitem = form['filename']

# Test if the file was uploaded
if fileitem.filename:
    # strip leading path from file name to avoid directory traversal attacks
    file_name = os.path.basename(fileitem.filename)
    upload_dir = os.path.join(os.getcwd(), "var/www/upload")
    file_path = os.path.join(upload_dir, file_name)
    if os.path.exists(file_path):
        message = 'Not uploaded: file with same name already present on server!'
        status_code = '409 Conflict'
    else:
        if not os.path.exists(upload_dir):
            os.makedirs(upload_dir)
        with open(file_path, 'wb') as fd:
            fd.write(fileitem.file.read())
        message = 'The file "' + file_name + '" was uploaded successfully'
        status_code = '201 Created'
        addLocation = True
else:
    message = 'No file found in request!'
    status_code = '400 Bad Request'

html_content = (
    "<!DOCTYPE html>\n"
    "<html><body>\n"
    "<h1>File upload</h1>\n"
    f"<p>{message}:</p>\n"
    f"<p>Current time: {now_str}</p>\n"
    "<a href='/'>go home</a>\n"
    "</body></html>\n"
)

# todo: validate headers
# if environ.has_key('SERVER_NAME'):

# print('Python debug line', file=sys.stderr)
# for name, value in environ.items():
# #     print("{0}: {1}".format(name, value), file=sys.stderr)

print(f"Status: {status_code}", end='\r\n')
print("Content-Type: text/html", end='\r\n')
print(f"Content-Length: {len(html_content)}", end='\r\n')
if (addLocation):
    print(f"Location: {os.path.join('/uploads', file_name)}", end='\r\n')
if ('SERVER_NAME' in environ.keys()):
    print(f"Server: {environ['SERVER_NAME']}", end='\r\n')
print("", end='\r\n')  # mandatory empty line
print(html_content)

# cgi.print_environ()
