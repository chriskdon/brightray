#ifndef _BRIGHTRAY_H
#define _BRIGHTRAY_H

typedef struct br_server br_server;

br_server* br_server_new();

int br_server_run(br_server *br);

// Config
void br_server_set_port(br_server *br, int port);

// Routing
void br_server_route_add(br_server *br, const char *route, const char *text);

#endif