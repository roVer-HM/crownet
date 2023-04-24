FROM gcc:latest

COPY ./client.cpp /usr/src/client.cpp

WORKDIR /usr/src/

RUN g++ -std=c++17 -Wall -Wextra -pedantic -o client client.cpp

CMD ["./client"]

#docker run -it --name cppclient --network rovernet  cppclient