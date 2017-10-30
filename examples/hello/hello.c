#include <brightray.h>

int handler(const br_request *req, br_response *res) {
  const char * path = br_request_path(req);

  res->status_code = 200;

  br_response_set_content_string(res, path);

  return 0;
}

int main() {
  br_server *br = br_server_new();

  br_server_set_port(br, 8080);

  br_server_route_add(br, "/", handler);
  br_server_route_add(br, "/test", handler);
  br_server_route_add(br, "/world", handler);

  return br_server_run(br);
}