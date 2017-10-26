#ifndef _BRIGHTRAY_H
#define _BRIGHTRAY_H

typedef struct {
  int port;
} brightray_t;

brightray_t* brightray_new();

int brightray_run(brightray_t *br);

#endif