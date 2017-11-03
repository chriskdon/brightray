#ifndef _BR_RESPONSE_H
#define _BR_RESPONSE_H

#include <stdlib.h>

typedef struct br_response {
  int status_code;

  char * header_fields;
  size_t header_fields_length;

  const char * content;
  size_t content_length;
} br_response;

void br_response_add_header(br_response * response, const char * field, const char * value);

void br_response_set_content_string(br_response * response, const char * str);

int br_response_to_buffer(br_response * r, char ** buffer, size_t * length);

#endif