### Compile
mkdir build
cd build
cmake ..
cmake --build .
./http_server




curl -X POST -d '{
    "FirstName": "Joe",
    "LastName": "Soap"
}' http://127.0.0.1:8080