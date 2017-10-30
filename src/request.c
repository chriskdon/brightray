#include "request.h"

const char * br_request_path(const br_request * request) {
  return request->path;
}
