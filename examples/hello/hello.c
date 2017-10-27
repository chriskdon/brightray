#include <brightray.h>

int handler(const br_request *req, br_response *res) {
  const char * path = br_request_path(req);

  br_response_set_content_string(res, path);

  return 0;
}

int default_handler(const br_request *req, br_response *res) {
  br_response_set_content_string(res, "Not Found");

  return 0;
}

int main() {
  br_server *br = br_server_new();

  br_server_set_port(br, 8080);

  br_server_route_add(br, "/", handler);
  br_server_route_add(br, "/test", handler);
  br_server_route_add(br, "/world", handler);

  br_server_route_default(br, default_handler);

  return br_server_run(br);
}