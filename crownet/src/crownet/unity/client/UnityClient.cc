#include "UnityClient.h"
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <omnetpp.h>
#include <arpa/inet.h>
#include <cctype>
#include <regex>
#include "json.hpp"
#include <thread>


Define_Module(UnityClient);
std::mutex UnityClient::m_mutex;
UnityClient* UnityClient::instance = nullptr;

void UnityClient::initialize() {
    UnityClient* unityClient = UnityClient::getInstance();
    std::regex regex("^[[:alpha:]]+$");
    char * HOST = "192.168.178.174";
    //char* HOST = "172.18.0.1";
    //char *HOST = "unityserver";
    int PORT = 12345;
    int connection_status;

    // create default IPv4 TCP socket
    unityClient->serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    printf("Successfully created server socket");
    // set up address and port. Accepts IpV4 connections from any address
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(12345);

    // since we are using a hostname instead of an ip address, we need to resolve.
    // gethostbyname() is obsolete, but sufficient as we don't requite IPv6
    // support or DNS.
    struct hostent *host_entry;

    // if the host contains only characters, we assume it's a hostname. Otherwise we parse the IP4 address.
    if (std::regex_match(HOST, regex)) {
        host_entry = gethostbyname(HOST);
        server_address.sin_addr = *((struct in_addr*) host_entry->h_addr);
    }
    else {
        server_address.sin_addr.s_addr = inet_addr(HOST);
    }
    printf("Successfully obtained address to connect");




    // connect to server
    connection_status = connect(unityClient->serverSocket,
            (struct sockaddr*) &server_address, sizeof(server_address));

    if (connection_status == 0) {
        printf("Successfully connected to the server!");
        // send a byte string to the server

        // char message[] = "Hello, server!";
        // int message_length = strlen(message);
        // int bytes_sent = ::send(serverSocket, message, message_length, 0);

//         if (bytes_sent == -1) {
//             printf("Error sending message to server\n");
//         } else {
//             printf("Sent %d bytes to server\n", bytes_sent);
//         }
    }

    cSimpleModule::initialize();

}

void UnityClient::finish() {
    UnityClient* unityClient = UnityClient::getInstance();
    std::cout << "Closed socket connection";
    close(unityClient->serverSocket);
    cSimpleModule::finish();
}

struct Message {
    inet::Coord coord;
    int id;
};

void UnityClient::sendMessage(int id, inet::Coord coord) {
    UnityClient* unityClient = UnityClient::getInstance();

    nlohmann::json data;
    data["id"] = 1;
    data["Coordinates"]["X"] = coord.x;
    data["Coordinates"]["Y"] = coord.y;
    data["Coordinates"]["Z"] = coord.z;


    std::string jsonStr = data.dump();
    jsonStr.append("<|EOM|>");
    const char* dataToSend = jsonStr.c_str();

    std::unique_lock<std::mutex> lock(unityClient->m_mutex);
        int result = ::send(unityClient->serverSocket, dataToSend, strlen(dataToSend), 0);
        printf("Message was send");
        if (result == -1) {
            perror("Error sending message:");
        }

    //lock.unlock();
}




void UnityClient::handleMessage(omnetpp::cMessage *msg) {
    cSimpleModule::handleMessage(msg);
}

// namespace crownet
