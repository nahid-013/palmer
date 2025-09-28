#include <asio.hpp>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <ctime>

using asio::ip::tcp;

std::string make_response(int status_code, const std::string& status_text,
                          const std::string& content_type,
                          const std::string& body) {
    std::ostringstream response;
    response << "HTTP/1.1 " << status_code << " " << status_text << "\r\n";
    response << "Content-Type: " << content_type << "\r\n";
    response << "Content-Length: " << body.size() << "\r\n";
    response << "Connection: close\r\n";
    response << "\r\n";
    response << body;
    return response.str();
}


std::string now_iso8601() {
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm utc_tm = *std::gmtime(&t);

    std::ostringstream oss;
    oss << std::put_time(&utc_tm, "%Y-%m-%dT%H:%M:%SZ");
    return oss.str();
}

int main() {
    try {
        asio::io_context ioc;
        tcp::acceptor acceptor{ioc, tcp::endpoint(tcp::v4(), 8080)};
        std::cout << "Listening on http://127.0.0.1:8080\n";

        for (;;) {
            tcp::socket socket{ioc};
            std::cout << "[WAIT] accept...\n";
            acceptor.accept(socket);
            std::cout << "[OK] accepted from " << socket.remote_endpoint() << "\n";

            asio::streambuf buf;
            std::cout << "[WAIT] read headers...\n";
            asio::read_until(socket, buf, "\r\n\r\n");
            std::cout << "[OK] headers read, bytes in buf = " << buf.size() << "\n";

            std::istream req(&buf);
            std::string request_line;
            std::getline(req, request_line);
            std::istringstream line_stream(request_line);
            std::string method, path, version;
            line_stream >> method >> path >> version;

            // читаем заголовки
            std::string header_line;
            std::size_t content_length = 0;
            while (std::getline(req, header_line) && header_line != "\r") {
                if (header_line.find("Content-Length:") == 0) {
                    content_length = std::stoul(header_line.substr(15));
                }
            }


            std::string body;
            if (content_length > 0) {

                std::size_t already_in_buf = buf.size();

                if (already_in_buf > 0) {
                    std::istream leftover(&buf);
                    std::string tmp(already_in_buf, '\0');
                    leftover.read(&tmp[0], already_in_buf);
                    body += tmp;
                }


                if (body.size() < content_length) {
                    std::size_t remaining = content_length - body.size();
                    std::string tmp(remaining, '\0');
                    asio::read(socket, asio::buffer(&tmp[0], remaining));
                    body += tmp;
                }
            }


            std::string response_body;
            int status = 200;
            std::string status_text = "OK";
            std::string content_type = "text/plain";

            if (method == "GET" && path == "/") {
                response_body = "OK";
            } else if (method == "GET" && path == "/time") {
                response_body = now_iso8601();
            } else if (method == "POST" && path == "/echo") {
                response_body = body;
            } else if (method == "GET" && path == "/health") {
                response_body = R"({"status":"up"})";
                content_type = "application/json";
            } else {
                status = 404;
                status_text = "Not Found";
                response_body = "Not Found";
            }

            std::string response = make_response(status, status_text, content_type, response_body);
            std::cout << "[SEND] response...\n";
            asio::write(socket, asio::buffer(response));
            std::cout << "[DONE] closing socket\n";

            asio::error_code ec;
            socket.shutdown(tcp::socket::shutdown_both, ec);
            socket.close(ec);
        }
    } catch (std::exception& e) {
        std::cerr << "Fatal: " << e.what() << "\n";
        return 1;
    }
}