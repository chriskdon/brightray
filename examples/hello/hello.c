#include <brightray.h>


int main() {
  brightray *br = brightray_new();

  brightray_set_port(br, 8080);

  brightray_route_add(br, "/", "Root");
  brightray_route_add(br, "/test", "Hello Test");
  brightray_route_add(br, "/world", "Hello World");

  return brightray_run(br);
}