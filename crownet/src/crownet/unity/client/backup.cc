//#include <netdb.h>
//#include <netinet/in.h>
//#include <stdio.h>
//#include <stdlib.h>
//#include <sys/socket.h>
//#include <sys/types.h>
//#include <unistd.h>
//
//int main(int argc, char const* argv[]) {
//  char* HOSTNAME = "cppserver";
//  int PORT = 12345;
//  int server_socket;
//  int connection_status;
//
//  // create default IPv4 TCP socket
//  server_socket = socket(AF_INET, SOCK_STREAM, 0);
//
//  // set up address and port. Accepts IpV4 connections from any address on $PORT
//  struct sockaddr_in server_address;
//  server_address.sin_family = AF_INET;
//  server_address.sin_port = htons(12345);
//
//  // since we are using a hostname instead of an ip address, we need to resolve.
//  // gethostbyname() is obsolete, but sufficient as we don't requite IPv6
//  // support or DNS.
//  struct hostent* host_entry;
//  host_entry = gethostbyname(HOSTNAME);
//  if (host_entry == NULL) {
//    printf("Error resolving hostname");
//  };
//
//  server_address.sin_addr = *((struct in_addr*)host_entry->h_addr);
//
//  // connect to server
//  connection_status = connect(server_socket, (struct sockaddr*)&server_address,
//                              sizeof(server_address));
//
//  if (connection_status == -1) {
//    printf("There was an error making a connection to the remote socket");
//  }
//  close(server_socket);
//  return 0;
//}
