#ifndef _BR_SERVER_TYPES_H
#define _BR_SERVER_TYPES_H

#include <stdlib.h>

#define MAX_HEADERS_COUNT 10

typedef struct br_http_header_s {
  char * field;
  char * value;
  size_t field_length;
  size_t value_length;
} br_http_header_t;

typedef struct br_request_s {
  const char * path;
} br_request_t;

typedef struct br_response_s {
  int status_code;

  br_http_header_t * headers[MAX_HEADERS_COUNT];
  int headers_length;

  char * content;
  size_t content_length;
} br_response_t;

typedef int (* br_handler_f)(const br_request_t * request, br_response_t * response);

typedef struct br_route_node_s {
  const char * route;
  br_handler_f handler;
  struct br_route_node_s * next;
  struct br_route_node_s * prev; 
} br_route_node_t;

typedef struct br_server_s {
  int port;
  br_route_node_t * routes_root;
  br_route_node_t * routes_last;
  br_handler_f default_handler;
} br_server_t;

#endif