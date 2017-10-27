#ifndef _BRIGHTRAY_H
#define _BRIGHTRAY_H

typedef struct br_server br_server;

typedef struct br_request br_request;
typedef struct br_response br_response;

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

// Request
const char * br_request_path(const br_request * request);

// Handlers
void br_response_set_content_string(br_response * response, const char * str);

#endif