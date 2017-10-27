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

#define MAXBUF 2048

typedef struct brightray_route_node {
  const char *route;
  const char *text;
  struct brightray_route_node *next; 
} brightray_route_node;

typedef struct br_server {
  int port;
  brightray_route_node *routes_root;
  brightray_route_node *routes_last;
} br_server;

volatile bool listen_for_connections = true;
int sockfd; // Listening socket

static void shutdown_server(int _){
  listen_for_connections = false;
  close(sockfd);
}

void br_server_set_port(br_server *br, int port) {
  br->port = port;
}

void br_server_route_add(br_server *br, const char *route, const char *text) {
  brightray_route_node *node = malloc(sizeof(brightray_route_node));

  node->route = route;
  node->text = text;
  node->next = NULL;

  if(br->routes_last == NULL) {
    br->routes_root = node;
  } else {
    br->routes_last->next = node;
  }

  br->routes_last = node;
}

br_server* br_server_new() {
  br_server *br = malloc(sizeof(br_server));

  br->port = 8080;
  br->routes_root = NULL;
  br->routes_last = NULL;

  return br;
}

int br_server_run(br_server *br) {
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

  // Template
  char * html_template = "HTTP/1.1 200 OK\r\n"
                         "Server: Brighray 0.0.1\r\n"
                         "Content-Length: %d\r\n"
                         "Content-Type: text/html\r\n"
                         "\r\n"
                         "%s";

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

    // Get the request
    recv(clientfd, buffer_recv, MAXBUF, 0);

    // Parse request
    char path[500];
    sscanf(buffer_recv,"GET %s HTTP/1.1", path);
    printf("Path: %s\n", path);

    // Find matching route handler
    brightray_route_node *handler = br->routes_root;  
    while(handler != NULL && strcmp(handler->route, path) != 0) {
      handler = handler->next;
    }

    // Send Response
    if(handler == NULL) {
      const char* test = "<b>Not Found</b>";
      int send_length = sprintf(buffer_send, html_template, strlen(test), test);
      write(clientfd, buffer_send, send_length);
    } else {
      int send_length = sprintf(buffer_send, html_template, strlen(handler->text), handler->text);
      write(clientfd, buffer_send, send_length);
    }

    // Close client socket
    close(clientfd);
  }

  printf("Server Shutdown Complete\n");

  return 0;
}