#ifndef _BR_RESPONSE_H
#define _BR_RESPONSE_H

#include <stdlib.h>

typedef struct br_response {
  const char * content;
  size_t content_length;
  int status_code;
} br_response;

void br_response_set_content_string(br_response * response, const char * str);

#endif