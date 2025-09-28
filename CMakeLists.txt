cmake_minimum_required(VERSION 3.16)
project(http_server CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# standalone режим
add_definitions(-DASIO_STANDALONE)

add_executable(http_server main.cpp)

# путь до asio из brew
target_include_directories(http_server PRIVATE /opt/homebrew/opt/asio/include)

# добавляем потоки
find_package(Threads REQUIRED)
target_link_libraries(http_server PRIVATE Threads::Threads)
