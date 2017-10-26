#include <brightray.h>

int handler_test() {
  return 0;
}

int main() {
  brightray *br = brightray_new();

  brightray_set_port(br, 8080);

  brightray_route_add(br, "test/", handler_test);

  return brightray_run(br);
}