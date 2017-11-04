#ifndef _BR_SERVER_H
#define _BR_SERVER_H

#include "request.h"
#include "response.h"

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

br_server_t * br_server_new();
void br_server_free(br_server_t * br);

// Run Server
int br_server_run(br_server_t * br);

// Config
void br_server_set_port(br_server_t * br, int port);

// Routing
void br_server_route_add(br_server_t * br, const char * route, const br_handler_f handler);
void br_server_route_default(br_server_t * br, const br_handler_f handler);

#endif