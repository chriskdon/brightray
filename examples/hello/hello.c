#include <brightray.h>

int main() {
  br_server *br = br_server_new();

  br_server_set_port(br, 8080);

  br_server_route_add(br, "/", "Root");
  br_server_route_add(br, "/test", "Hello Test");
  br_server_route_add(br, "/world", "Hello World");

  return br_server_run(br);
}