#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <errno.h>
#include <sys/socket.h>
#include <resolv.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdbool.h>

#include <uv.h>

#include <sds.h>

#include "brightray.h"
#include "debug.h"

#define MAXBUF 2048
#define MAX_PATH_LENGTH 256

#define UV_ERR(err, msg) fprintf(stderr, "%s: %s\n", msg, uv_strerror(err))

#define UV_CHECK(err, msg) \
do { \
  if (err != 0) { \
    UV_ERR(err, msg); \
    exit(1); \
  } \
} while(0)

#define MAX_WRITE_HANDLES 1000

static uv_loop_t * uv_loop;
static uv_tcp_t server;

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

int br_default_handler(const br_request *req, br_response *res) {
  res->status_code = 404;
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

/**
 * Parse the path from the HTTP request.
 * 
 * The path will be stored in the 'path' paramaeter. It is the responsibility
 * of the caller to free 'path'.
 */
br_error br__parse_path(const char * request, sds * path) {
  *path = sdsempty();
  
  int i = 0;
  
  // Skip request method
  while(request[i] != ' ') { i++; } 
  i++;

  // Get request path
  int length;
  for(length = 0; request[i] != ' ' && length <= MAX_PATH_LENGTH; i++, length++) {
    *path = sdscatlen(*path, &request[i], 1);
  }

  if(request[i] != ' ' && length > MAX_PATH_LENGTH) {
    return BR_ERROR_GENERIC;
  }

  return BR_SUCCESS;
}

void on_connect(uv_stream_t *server_handle, int status) {

}

int br_server_run(br_server * br) {
  struct sockaddr_in address;

  signal(SIGPIPE, SIG_IGN);

  uv_loop = uv_default_loop();

  UV_CHECK(uv_tcp_init(uv_loop, &server), "tcp_init");
  UV_CHECK(uv_ip4_addr("0.0.0.0", 8080, &address), "ip4_addr");
  UV_CHECK(uv_tcp_bind(&server, (const struct sockaddr *) &address, 0), "tcp_bind");
  UV_CHECK(uv_listen((uv_stream_t *) &server, MAX_WRITE_HANDLES, on_connect), "uv_listen");
  
  printf("Listening on: 0.0.0.0:8080\n");

  return uv_run(uv_loop, UV_RUN_DEFAULT);
}