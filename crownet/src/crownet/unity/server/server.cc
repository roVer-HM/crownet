//#include <netinet/in.h>
//#include <stdio.h>
//#include <stdlib.h>
//#include <sys/socket.h>
//#include <sys/types.h>
//
//int main() {
//  // TODO: READ PORT FROM NED FILE PARAMETER
//  // TODO: SEND WELCOME MESSAGE TO SOCKET
//  int PORT = 12345;
//  int server_socket;
//  int client_socket;
//
//  // create default IPv4 TCP socket
//  server_socket = socket(AF_INET, SOCK_STREAM, 0);
//
//  // set up address and port. Accepts IpV4 connections from any address on $PORT
//  struct sockaddr_in server_address;
//  server_address.sin_family = AF_INET;
//  server_address.sin_port = htons(PORT);
//  server_address.sin_addr.s_addr = INADDR_ANY;
//
//  // bind socket to specified address and port
//  bind(server_socket, (struct sockaddr*)&server_address,
//       sizeof(server_address));
//
//  // listen for connections
//  listen(server_socket, 5);
//
//  // accept connection
//  client_socket = accept(server_socket, NULL, NULL);
//
//  if (client_socket == -1) {
//    printf("There was an error making a connection to the remote socket");
//  }
//
//  return 0;
//}
