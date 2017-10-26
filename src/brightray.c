#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <errno.h>
#include <sys/socket.h>
#include <resolv.h>
#include <arpa/inet.h>

#define MY_PORT   9090
#define MAXBUF    1024

int brightray_run() {
  //printf("Starting Brightray");

  int sockfd;
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
  self.sin_port         = htons(MY_PORT);
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
  while(1) {
    int clientfd;
    struct sockaddr_in client_addr;
    int addrlen = sizeof(client_addr);

    // Accept connection
    clientfd = accept(sockfd, (struct sockaddr*)&client_addr, &addrlen);
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

  // Cleanup
  close(sockfd);
  return 0;
}