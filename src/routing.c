#include <assert.h>

#include "routing.h"

void br_routes_add(br_server_t * br, const char * route, const br_handler_f handler) {
  assert(br != NULL);
  assert(route != NULL);
  assert(handler != NULL);

  br_route_node_t * node = malloc(sizeof(br_route_node_t));

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

void br_routes_default(br_server_t * br, const br_handler_f handler) {
  assert(br != NULL);
  assert(handler != NULL);

  br->default_handler = handler;
}