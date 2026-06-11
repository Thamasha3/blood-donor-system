#ifndef LIGHTWEIGHT_HTTPLIB_H
#define LIGHTWEIGHT_HTTPLIB_H

#include <iostream>
#include <string>
#include <sstream>
#include <map>
#include <functional>
#include <winsock2.h>

namespace httplib {

    struct Request {
        std::string method;
        std::string path;
        std::string body;
    };

    struct Response {
        int status = 200;
        std::map<std::string, std::string> headers;
        std::string body;

        void set_header(const std::string& key, const std::string& val) {
            headers[key] = val;
        }

        void set_content(const std::string& content, const std::string& content_type) {
            body = content;
            set_header("Content-Type", content_type);
        }
    };

    class Server {
    private:
        std::map<std::string, std::function<void(const Request&, Response&)>> get_handlers;
        std::map<std::string, std::function<void(const Request&, Response&)>> post_handlers;
        std::function<void(const Request&, Response&)> options_handler;

    public:
        Server() {
            WSADATA wsaData;
            WSAStartup(MAKEWORD(2, 2), &wsaData);
        }

        ~Server() {
            WSACleanup();
        }

        void Get(const std::string& path, std::function<void(const Request&, Response&)> handler) {
            get_handlers[path] = handler;
        }

        void Post(const std::string& path, std::function<void(const Request&, Response&)> handler) {
            post_handlers[path] = handler;
        }

        void set_to_options(std::function<void(const Request&, Response&)> handler) {
            options_handler = handler;
        }

        void listen(const std::string& host, int port) {
            SOCKET server_fd = socket(AF_INET, SOCK_STREAM, 0);
            if (server_fd == INVALID_SOCKET) return;

            // Allow quick reuse of the local network port during testing refreshes
            char opt = 1;
            setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

            sockaddr_in address;
            address.sin_family = AF_INET;
            address.sin_addr.s_addr = INADDR_ANY;
            address.sin_port = htons(port);

            if (bind(server_fd, (struct sockaddr*)&address, sizeof(address)) == SOCKET_ERROR) return;
            if (::listen(server_fd, 10) == SOCKET_ERROR) return;

            while (true) {
                int addrlen = sizeof(address);
                SOCKET client_socket = accept(server_fd, (struct sockaddr*)&address, &addrlen);
                if (client_socket == INVALID_SOCKET) continue;

                char buffer[4096] = {0};
                int valread = recv(client_socket, buffer, 4096, 0);
                if (valread <= 0) {
                    closesocket(client_socket);
                    continue;
                }

                std::string raw_request(buffer, valread);
                std::stringstream ss(raw_request);
                Request req;
                ss >> req.method >> req.path;

                // Extract body contents for POST requests
                size_t body_pos = raw_request.find("\r\n\r\n");
                if (body_pos != std::string::npos) {
                    req.body = raw_request.substr(body_pos + 4);
                }

                Response res;

                // Handle Pre-flight security OPTIONS request from modern browsers
                if (req.method == "OPTIONS" && options_handler) {
                    options_handler(req, res);
                }
                else if (req.method == "GET" && get_handlers.find(req.path) != get_handlers.end()) {
                    get_handlers[req.path](req, res);
                }
                else if (req.method == "POST" && post_handlers.find(req.path) != post_handlers.end()) {
                    post_handlers[req.path](req, res);
                } else {
                    res.status = 404;
                    res.set_content("{\"error\":\"Endpoint path not mapped inside C++ server\"}", "application/json");
                }

                // Construct standard clean HTTP output response string
                std::stringstream out;
                out << "HTTP/1.1 " << res.status << " OK\r\n";
                for (auto const& h : res.headers) {
                    out << h.first << ": " << h.second << "\r\n";
                }
                out << "Content-Length: " << res.body.length() << "\r\n";
                out << "Connection: close\r\n\r\n";
                out << res.body;

                std::string response_str = out.str();
                send(client_socket, response_str.c_str(), response_str.length(), 0);
                closesocket(client_socket);
            }
        }
    };
}

#endif
