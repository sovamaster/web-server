
Simple Web-Server and WebSocket-Server
=================

A very simple, fast, multithreaded web and websocket server. Easy to use to create C ++ REST and WEB applications. 


### Features

* Multithreaded request handling
* Platform independent
* HTTPS clients support
* HTTP persistent connection (for HTTP/1.1)
* Client supports chunked transfer encoding
* Synchronous handling of multiple websocket clients

### Usage

See test.hpp and api.hpp for example usage. 

### Dependencies

* For json data handling: JSONCONS - https://github.com/danielaparker/jsoncons
* For HTTPS: OpenSSL libraries 

### Compile and run

Compile with a C++11 compliant compiler:
```sh
cd <your project directory> //At the same level as the Makefile.
make server
./web-server
```

#### Checks and testing

For web-server testing open with your favorite browser swagger page  http://localhost:37378/swagger/ui
For websocket-server testing open with your favorite browser test page  http://localhost:37378/ws_test.html

If your test domain is different from localhost, change it in line 35 in the ws_test.html file.
If you want to change port number, do it in the web-server.ini file. For changing websocket port also change one in line 35 in the ws_test.html file.

To test the web client, the https://httpbin.org/ REST service is used.