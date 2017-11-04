#ifndef _BR_ROUTING_H
#define _BR_ROUTING_H

#include "types.h"

void br_routes_add(br_server_t * br, const char * route, const br_handler_f handler);

void br_routes_default(br_server_t * br, const br_handler_f handler);

#endif