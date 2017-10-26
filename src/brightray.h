#ifndef _BRIGHTRAY_H
#define _BRIGHTRAY_H

typedef struct brightray brightray;

brightray* brightray_new();

int brightray_run(brightray *br);

// Config
void brightray_set_port(brightray *br, int port);

// Routing
void brightray_route_add(brightray *br, const char* route, int (*handler)());

#endif