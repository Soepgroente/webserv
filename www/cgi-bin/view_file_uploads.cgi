#!/usr/bin/python3
import sys
from os import environ, listdir
from pathlib import Path

html_file_section = ""
fetch_script = "<script>function deleteRequest(path) {fetch(path, {method: 'DELETE'});}</script>"

p_www = ""

path_tmp = Path(environ['SCRIPT_FILENAME']).parent
while (path_tmp.__str__() != path_tmp.root):
    path_tmp = path_tmp.parent
    if (path_tmp.name == "www"):
        p_www = path_tmp

if (p_www != "" and (p_www / 'upload').is_dir()):
    upload_dir = (p_www / 'upload').__str__()
    file_list = listdir(path=upload_dir)
    if (len(file_list) == 0):
        html_file_section += "<p>There are no uploaded files on the server</p>"
    else:
        html_file_section += "<ul>\n"
        for file_name in listdir(path=upload_dir):
            delete_button = f'<input type="button" value="Delete" onclick="deleteRequest(\'/upload/{file_name}\');">'
            html_file_section += f"\t<li>{file_name} {delete_button}</li>\n"
        html_file_section += "</ul>\n"
else:
    html_file_section += "<p>There are no uploaded files on the server</p>"

html_content = (
    "<!DOCTYPE html>\n"
    "<html><body>\n"
    f"{fetch_script}\n"
    "<h1>Uploaded files</h1>\n"
    "<a href='/'>go home</a>\n"
    f"{html_file_section}\n"
    "</body></html>\n"
)

# print('Python debug line', file=sys.stderr)
# for name, value in environ.items():
#     print("{0}: {1}".format(name, value), file=sys.stderr)

print(f"Status: 200 OK", end='\r\n')
print("Content-Type: text/html", end='\r\n')
print(f"Content-Length: {len(html_content)}", end='\r\n')
if ('SERVER_NAME' in environ.keys()):
    print(f"Server: {environ['SERVER_NAME']}", end='\r\n')
print("", end='\r\n')  # mandatory empty line
print(html_content)
