#include <asio.hpp>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <ctime>
// #include "help_services/time.h"


using namespace std;
using asio::ip::tcp;


struct HttpRequest {
    std::string method;
    std::string path;
    std::string version;
    std::size_t content_length = 0;
    std::string body;
};


// ВСПОМОГАТЕЛЬНЫЕ 
std::string make_response(int status_code, const std::string& status_text, const std::string& content_type, const std::string& body) {
    std::ostringstream response;
    response << "HTTP/1.1 " << status_code << " " << status_text << "\r\n";
    response << "Content-Type: " << content_type << "\r\n";
    response << "Content-Length: " << body.size() << "\r\n";
    response << "Connection: close\r\n";
    response << "\r\n";
    response << body;
    return response.str();
}

std::string get_time() {
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm utc_tm = *std::gmtime(&t);

    std::ostringstream oss;
    oss << std::put_time(&utc_tm, "%Y-%m-%dT%H:%M:%SZ");
    return oss.str();
}

std::string handle_root() {
    return make_response(200, "OK", "text/plain", "OK");
}

std::string handle_time() {
    return make_response(200, "OK", "text/plain", get_time());
}

std::string handle_echo(const HttpRequest& req) {
    return make_response(200, "OK", "text/plain", req.body);
}

std::string handle_health() {
    return make_response(200, "OK", "application/json", R"({"status":"up"})");
}

std::string handle_not_found() {
    return make_response(404, "Not Found", "text/plain", "Not Found");
}





// ОСНОВНЫЕ
tcp::socket accept_connection(asio::io_context& ioc, tcp::acceptor& acceptor) {
    tcp::socket socket{ioc};
    std::cout << "[WAIT] accept...\n";
    acceptor.accept(socket);
    std::cout << "[OK] accepted from " << socket.remote_endpoint() << "\n";
    return socket;
}

HttpRequest read_request(tcp::socket& socket) {


    asio::streambuf buf;
    std::cout << "[WAIT] read headers...\n";
    asio::read_until(socket, buf, "\r\n\r\n");
    std::cout << "[OK] headers read, bytes in buf = " << buf.size() << "\n";

    HttpRequest req;

    {
        std::istream stream(&buf);
        std::string request_line;
        std::getline(stream, request_line);
        std::istringstream line_stream(request_line);
        line_stream >> req.method >> req.path >> req.version;

        std::string header_line;
        while (std::getline(stream, header_line) && header_line != "\r") {
            if (header_line.find("Content-Length:") == 0) {
                req.content_length = std::stoul(header_line.substr(15));
            }
        }

        if (req.content_length > 0) {
            std::size_t already_in_buf = buf.size();
            if (already_in_buf > 0) {
                std::string tmp(already_in_buf, '\0');
                stream.read(&tmp[0], already_in_buf);
                req.body += tmp;
            }

            if (req.body.size() < req.content_length) {
                std::size_t remaining = req.content_length - req.body.size();
                std::string tmp(remaining, '\0');
                asio::read(socket, asio::buffer(&tmp[0], remaining));
                req.body += tmp;
            }
        }
    }

    return req;
}

std::string handle_request(const HttpRequest& req) {
    if (req.method == "GET" && req.path == "/") {
        return handle_root();
    } else if (req.method == "GET" && req.path == "/time") {
        return handle_time();
    } else if (req.method == "POST" && req.path == "/echo") {
        return handle_echo(req);
    } else if (req.method == "GET" && req.path == "/health") {
        return handle_health();
    } else {
        return handle_not_found();
    }
}

void send_response(tcp::socket& socket, const std::string& response) {
    std::cout << "[SEND] response...\n";
    asio::write(socket, asio::buffer(response));
    std::cout << "[DONE] closing socket\n";

    asio::error_code ec;
    socket.shutdown(tcp::socket::shutdown_both, ec);
    socket.close(ec);
}

int http_server_run(int port) {
    try {
        asio::io_context ioc;
        tcp::acceptor acceptor{ioc, tcp::endpoint(tcp::v4(), port)};
        std::cout << "Listening on http://127.0.0.1:" << port << "\n";

        for (;;) {
            auto socket = accept_connection(ioc, acceptor);
            auto req = read_request(socket);
            auto resp = handle_request(req);
            send_response(socket, resp);
        }
    } catch (std::exception& e) {
        std::cerr << "Fatal: " << e.what() << "\n";
        return 1;
    }
}



int main() {
    http_server_run(8080);
}