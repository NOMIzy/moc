#include "test.h"

#include <clock.h>

int main() {
  moc::clock c;
  while (true) {
    auto used = c.tick(1000);
    printf("last frame computed for %d ms\n", used);
  }
  return 0;
}
