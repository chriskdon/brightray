#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <errno.h>
#include <sys/socket.h>
#include <resolv.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdbool.h>

#include "brightray.h"

#define MAXBUF    1024

volatile bool listen_for_connections = true;
int sockfd; // Listening socket

static void shutdown_server(int _){
  listen_for_connections = false;
  close(sockfd);
}

brightray_t* brightray_new() {
  brightray_t *br = malloc(sizeof(brightray_t));

  return br;
}

int brightray_run(brightray_t *br) {
  signal(SIGINT,shutdown_server);
  signal(SIGTERM,shutdown_server);
  
  printf("Starting Brightray: http://localhost:%d\n", br->port);
  
  struct sockaddr_in self;
  char buffer_recv[MAXBUF];
  char buffer_send[MAXBUF];

  // Create streaming socket
  if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    perror("Socket");
    exit(errno);
  }

  // Initialize address/port struct
  bzero(&self, sizeof(self));
  self.sin_family       = AF_INET;
  self.sin_port         = htons(br->port);
  self.sin_addr.s_addr  = INADDR_ANY;

  // Assign a port number to the socket
  if(bind(sockfd, (struct sockaddr*)&self, sizeof(self)) != 0) {
    perror("socket--bind");
    exit(errno);
  }

  // Make it a listening socket
  if(listen(sockfd, 20) != 0) {
    perror("socket--listen");
    exit(errno);
  }

  // Listen for connections
  while(listen_for_connections) {
    int clientfd;
    struct sockaddr_in client_addr;
    int addrlen = sizeof(client_addr);

    // Accept connection
    clientfd = accept(sockfd, (struct sockaddr*)&client_addr, (socklen_t*)&addrlen);
    
    // The socket was closed when there were no connections
    if(!listen_for_connections) { break; }
    
    printf("%s:%d connected\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

    //int length = recv(clientfd, buffer_recv, MAXBUF, 0);

    // Send response
    char * sendstr = "HTTP/1.0 200 OK\r\n"
                     "Date: Sun, 18 Oct 2009 08:56:53 GMT\r\n"
                     "Server: Brighray 0.0.1\r\n"
                     "Content-Length: 12\r\n"
                     "Content-Type: text/html\r\n"
                     "\r\n"
                     "Hello world!";

    write(clientfd, sendstr, strlen(sendstr));

    // Close
    close(clientfd);
  }

  printf("Server Shutdown Complete\n");

  return 0;
}