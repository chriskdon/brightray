#include <strings.h>

#include "response.h"

void br_response_set_content_string(br_response * response, const char * str) {
  response->content = str;
  response->content_length = strlen(str);
}
