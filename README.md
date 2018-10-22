								HTTP Web Server

Author : Kamal Chaturvedi
Email  : kamalchaturvedi15@gmail.com
To build the Web Server , open shell and type the following command

> make

To run the server on a specified port say 8080, type

> ./webServer 8080

:: Details ::

Http Web server will have a document root which would contain files with the extension “.html” “.htm”, “.txt”, “.jpg”, “.gif”, ".js",&  ".css". When the web-server receives a HTTP request from a client (from a web-browser such as chrome) for a particular file, it opens the file from this document root and then send the file back to the client with proper header information.

Default Page:
If the Request URL is the directory itself, the web server tries to find a default web page such as “index.html” or “index.htm” on the requested directory. What this means is that when no file is requested in the URL and just the directory is requested (Example: GET / HTTP/1.1 or GET /inside/ HTTP/1.1), then a default web-page should be displayed. 

Handling Multiple Connections:
When the client receives a web page that contains a lot of embedded pictures, it repeatedly requests an
HTTP GET message for each object to the server. In such a case the server should be capable of serving
multiple requests at same point of time. This has been enabled via forking (multi-process approach) for each incoming request.

Error Handling:
When the HTTP request results in an error then the web-server should respond to the client with an error
code. All error messages can be treated as the “500 Internet Server Error” indicating that
the server experiences unexpected system errors, except the "404 Not Found" error when client requests a file server doesnt even have

POST method support:
The code also handles POST requests for ‘.html’ files. When you send the POST request,
the new data needs to be appended to the existing HTML file and the same file should be returned, as for the GET request
The data to be posted is everything in the POST request, following the first blank line.




