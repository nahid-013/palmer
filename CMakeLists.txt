cmake_minimum_required(VERSION 3.16)
project(http_server CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


add_definitions(-DASIO_STANDALONE)

add_executable(http_server 
src/main.cpp 
src/server/time.cpp
src/server/response.cpp
)


# target_include_directories(http_server PRIVATE /opt/homebrew/opt/asio/include)
target_include_directories(http_server
    PRIVATE 
        ${CMAKE_SOURCE_DIR}/include
        /opt/homebrew/opt/asio/include
)

find_package(Threads REQUIRED)
target_link_libraries(http_server PRIVATE Threads::Threads)
