#ifndef _BR_RESPONSE_H
#define _BR_RESPONSE_H

#include "types.h"

br_response_t * br_response_new();
void br_response_free(br_response_t * response);

void br_response_add_header(br_response_t * response, const char * field, const char * value);

void br_response_set_content_string(br_response_t * response, const char * str);

int br_response_to_buffer(br_response_t * r, char ** buffer, size_t * length);

#endif