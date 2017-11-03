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
#include <http_parser.h>
#include <assert.h>

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
#define MAX_HTTP_HEADERS 20

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

typedef struct {
  char *field;
  char *value;
  size_t field_length;
  size_t value_length;
} http_header_t;

typedef struct {
  uv_write_t req;
  uv_stream_t stream;
  http_parser parser;
  char *url;
  char *method;
  int header_lines;
  http_header_t headers[MAX_HTTP_HEADERS];
  char *body;
  size_t body_length;
  uv_buf_t resp_buf[2];
} http_request_t;

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
  UV_CHECK(status, "connect");

  assert((uv_tcp_t *) server_handle == &server);

  http_request_t * http_request = malloc(sizeof(http_request_t));

  uv_tcp_init(uv_loop, (uv_tcp_t *) &http_request->stream);

  http_request->stream.data = http_request;
  http_request->parser.data = http_request;
  http_request->req.data = http_request;
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