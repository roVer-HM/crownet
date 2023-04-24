#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <cstring>
#include <iostream>

#include "crownet/unity/visualizer/CrownetMobilityVisualizer.h"

namespace crownet {

Define_Module(CrownetMobilityVisualizer);

void CrownetMobilityVisualizer::initialize(int stage) {
  MobilityVisualizerBase::initialize(stage);
  if (stage == inet::INITSTAGE_APPLICATION_LAYER) {

    std::cout << "Test\n";


    //TODO: read ip and port from ned file parameter to support connection from outside the docker network.

    const char* host = "pythonserver";
    const unsigned short port = 12345;

    // create IPv4 TCP socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        std::cerr << "Error creating socket" << std::endl;
    }

    struct sockaddr_in server_addr;

    // initalize struct with zeros to prevent unexpected behavior due to undefined or residual data
    std::memset(&server_addr, 0, sizeof(server_addr));
    // specify address family as IPv4
    server_addr.sin_family = AF_INET;
    // store byte representation of port
    server_addr.sin_port = htons(port);
    // store byte representation of address
    inet_pton(AF_INET, host, &server_addr.sin_addr);

    // attempt to connect to specified address and port
    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        std::cerr << "Error connecting to the server" << std::endl;
        perror("connect");
        close(sockfd);
    }


    std::cout << "Connected to " << host << ":" << port << std::endl;

















    // par('foo');
    //  Add your custom initialization code if needed
  }
}

void CrownetMobilityVisualizer::receiveSignal(cComponent *source,
                                              inet::simsignal_t signal,
                                              cObject *object,
                                              cObject *details) {
  MobilityVisualizerBase::receiveSignal(source, signal, object, details);
  // Add your custom signal handling code if needed
}

void CrownetMobilityVisualizer::handleParameterChange(const char *name) {
  MobilityVisualizerBase::handleParameterChange(name);
  // Add your custom parameter handling code if needed
}

void CrownetMobilityVisualizer::subscribe() {
  MobilityVisualizerBase::subscribe();
  // Add your custom subscription code if needed
}

void CrownetMobilityVisualizer::unsubscribe() {
  MobilityVisualizerBase::unsubscribe();
  // Add your custom unsubscription code if needed
}

CrownetMobilityVisualizer::MobilityVisualization *
CrownetMobilityVisualizer::createMobilityVisualization(
    inet::IMobility *mobility) {
  auto visualization =
      new CrownetMobilityVisualizer::MobilityVisualization(mobility);
  return visualization;
}

void CrownetMobilityVisualizer::removeMobilityVisualization(
    const MobilityVisualization *visualization) {
  MobilityVisualizerBase::removeMobilityVisualization(visualization);
  // Add your custom remove MobilityVisualization code if needed
}
}  // namespace crownet
