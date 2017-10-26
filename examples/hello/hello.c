#include <brightray.h>

int main() {
  brightray_t *br = brightray_new();

  br->port = 8080;

  return brightray_run(br);
}