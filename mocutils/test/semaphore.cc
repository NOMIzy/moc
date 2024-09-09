#include <semaphore.h>
#include "test.h"

int main() {{
  moc::semaphore<3> s(2);
  s.release();
  s.acquire();
  s.acquire();
  s.acquire();
  s.release();
  s.release();
  test(s.val == 2);
  pass("semaphore with init val");
}; {
  moc::semaphore<3> s;
  s.release();
  s.acquire();
  s.release();
  s.release();
  s.release();
  s.acquire();
  s.acquire();
  s.release();
  s.release();
  test(s.val == 3);
  pass("semaphore without init val");
} return 0;
}
