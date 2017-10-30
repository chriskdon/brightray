#ifndef _BR_REQUEST_H
#define _BR_REQUEST_H

typedef struct br_request {
  const char * path;
} br_request;

const char * br_request_path(const br_request * request);

#endif