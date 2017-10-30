#ifndef _BRIGHTRAY_H
#define _BRIGHTRAY_H

#include "response.h"
#include "request.h"

typedef struct br_server br_server;

typedef int (* br_handler)(const br_request * request, br_response * response);

br_server * br_server_new();
void br_server_free(br_server * br);

// Run Server
int br_server_run(br_server * br);

// Config
void br_server_set_port(br_server * br, int port);

// Routing
void br_server_route_add(br_server * br, const char * route, const br_handler handler);
void br_server_route_default(br_server * br, const br_handler handler);

#endif