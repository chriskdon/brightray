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
  const char * route;
  br_handler handler;
  struct brightray_route_node * next;
  struct brightray_route_node * prev; 
} brightray_route_node;

typedef struct br_server {
  int port;
  brightray_route_node * routes_root;
  brightray_route_node * routes_last;
  br_handler default_handler;
} br_server;

typedef struct br_request {
  const char * path;
} br_request;

typedef struct br_response {
  const char * content;
} br_response;

volatile bool listen_for_connections = true;
int sockfd; // Listening socket

static void shutdown_server(int _){
  listen_for_connections = false;
  close(sockfd);
}

void br_server_set_port(br_server *br, int port) {
  br->port = port;
}

void br_server_route_add(br_server *br, const char *route, const br_handler handler) {
  brightray_route_node *node = malloc(sizeof(brightray_route_node));

  node->route = route;
  node->handler = handler;
  node->next = NULL;
  node->prev = br->routes_last;

  if(br->routes_last == NULL) {
    br->routes_root = node;
  } else {
    br->routes_last->next = node;
  }

  br->routes_last = node;
}

void br_server_route_default(br_server * br, const br_handler handler) {
  br->default_handler = handler;
}

const char * br_request_path(const br_request * request) {
  return request->path;
}

void br_response_set_content_string(br_response * response, const char * str) {
  response->content = str;
}

int br_default_handler(const br_request *req, br_response *res) {
  br_response_set_content_string(res, "Not Found");

  return 0;
}

br_server * br_server_new() {
  br_server * br = malloc(sizeof(br_server));

  br->port = 8080;
  br->routes_root = NULL;
  br->routes_last = NULL;
  br->default_handler = br_default_handler;

  return br;
}

void br_server_free(br_server * br) {
  for(brightray_route_node *node = br->routes_last; node != NULL; node = node->prev)
  {
    free(node);
  }

  free(br);
}

int br_server_run(br_server * br) {
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

    // Fill Request
    br_request request = {
      .path = path
    };

    // Find matching route route
    brightray_route_node *route = br->routes_root;  
    while(route != NULL && strcmp(route->route, path) != 0) {
      route = route->next;
    }

    // Get handler
    br_handler handler;

    if(route == NULL) {
      handler = br->default_handler;
    } else {
      handler = route->handler;
    }

    // Send response
    br_response response;
    
    if(handler(&request, &response) != 0) {
      perror("handler--error");
      exit(-1);
    }

    int send_length = sprintf(buffer_send, html_template, strlen(response.content), response.content);
    write(clientfd, buffer_send, send_length);

    // Close client socket
    close(clientfd);
  }

  br_server_free(br);

  printf("Server Shutdown Complete\n");

  return 0;
}