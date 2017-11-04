#include "request.h"

const char * br_request_path(const br_request_t * request) {
  return request->path;
}
