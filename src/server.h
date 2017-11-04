#ifndef _BR_SERVER_H
#define _BR_SERVER_H

#include "types.h"
#include "request.h"
#include "response.h"
#include "routing.h"

br_server_t * br_server_new();
void br_server_free(br_server_t * br);

// Run Server
int br_server_run(br_server_t * br);

// Config
void br_server_set_port(br_server_t * br, int port);

#endif