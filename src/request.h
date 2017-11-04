#ifndef _BR_REQUEST_H
#define _BR_REQUEST_H

typedef struct br_request_s {
  const char * path;
} br_request_t;

const char * br_request_path(const br_request_t * request);

#endif