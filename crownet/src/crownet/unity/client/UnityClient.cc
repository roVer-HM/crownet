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

static std::string getIdFromPath(const std::string& input) {
    auto firstDot = input.find('.');
    if (firstDot == std::string::npos) {
        return input;
    }

    auto secondDot = input.find('.', firstDot + 1);
    return secondDot == std::string::npos ? input.substr(firstDot + 1) : input.substr(firstDot + 1, secondDot - firstDot - 1);
}

void UnityClient::initialize() {
    auto* unityClient = UnityClient::getInstance();
    std::regex regex("^[[:alpha:]]+$");
    const char* HOST = par("hostAddress").stringValue();
    int PORT = par("port").intValue();

    unityClient->serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (unityClient->serverSocket < 0) {
        perror("Failed to create server socket");
        exit(EXIT_FAILURE);
    }
    printf("Successfully created server socket\n");

    struct sockaddr_in server_address = {0};
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);

    struct hostent* host_entry;
    if (std::regex_match(HOST, regex)) {
        host_entry = gethostbyname(HOST);
        if (!host_entry) {
            perror("Failed to resolve hostname");
            exit(EXIT_FAILURE);
        }
        server_address.sin_addr = *((struct in_addr*)host_entry->h_addr);
    } else {
        server_address.sin_addr.s_addr = inet_addr(HOST);
        if (server_address.sin_addr.s_addr == INADDR_NONE) {
            perror("Invalid IP address");
            exit(EXIT_FAILURE);
        }
    }
    printf("Successfully obtained address to connect\n");

    if (connect(unityClient->serverSocket, (struct sockaddr*)&server_address, sizeof(server_address)) < 0) {
        perror("Failed to connect to the server");
        exit(EXIT_FAILURE);
    }
    printf("Successfully connected to the server!\n");

    cSimpleModule::initialize();
}

void UnityClient::finish() {
    auto* unityClient = UnityClient::getInstance();
    close(unityClient->serverSocket);
    printf("Closed socket connection\n");
    cSimpleModule::finish();
}

void UnityClient::sendMessage(const std::string& sourceId, const std::string& objectType, inet::Coord coordinates, const std::string& targetId) {
    auto* unityClient = UnityClient::getInstance();

    std::string sourcePath = getIdFromPath(sourceId);
    std::string targetPath = getIdFromPath(targetId);
    simtime_t currentSimTime = simTime();
    nlohmann::json data = {
        {"SourceId", sourcePath},
        {"TargetId", targetPath},
        {"Coordinates", {{"X", coordinates.x}, {"Y", coordinates.y}, {"Z", coordinates.z}}},
        {"ObjectType", objectType},
        {"SimTime", currentSimTime.str()}
    };

    std::string jsonStr = data.dump();
    const char* dataToSend = jsonStr.c_str();
    uint32_t length = htonl(jsonStr.size());

    ::send(unityClient->serverSocket, &length, sizeof(length), 0);
    ::send(unityClient->serverSocket, dataToSend, jsonStr.size(), 0);
}

void UnityClient::handleMessage(omnetpp::cMessage* msg) {
    cSimpleModule::handleMessage(msg);
}
