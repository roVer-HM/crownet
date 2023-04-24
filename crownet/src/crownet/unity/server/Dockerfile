FROM gcc:latest

COPY ./server.cpp /usr/src/server.cpp
WORKDIR /usr/src/

EXPOSE 12345


RUN g++ -std=c++17 -Wall -Wextra -pedantic -o server server.cpp

CMD ["./server"]

#docker run -p 12345:12345 --network rovernet -it --name cppserver cppserver
