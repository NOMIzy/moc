#include "channel.h"
#include "test.h"

using namespace std;
using namespace moc;

int main() {{
  auto c = make(channel, int, 3);
  test(typeid(c).name() == typeid(bchannel<int, 3>).name());
  pass("make macro for bchannel");

  test(c.cap == 3);
  c << 1;
  test(c.size == 1);
  c << 2;
  test(c.size == 2);
  c << 3;
  test(c.size == 3);
  int recv;
  c >> recv;
  test(c.size == 2);
  test(recv == 1);
  c >> recv;
  test(c.size == 1);
  test(recv == 2);
  c >> recv;
  test(c.size == 0);
  test(recv == 3);
  pass("bchannel");
}; {
  auto c = make(channel, int);
  test(typeid(c).name() == typeid(nbchannel<int>).name());
  pass("make macro for nbchannel");

  test(c.size == 0);
  test(c.cap == 0);
  pass("nbchannel");
}; {
  auto c0 = makeptr(channel, int, 3);
  auto c1 = new bchannel<int, 3>;
  test(typeid(c0).name() == typeid(c1).name());
  pass("makeptr macro for bchannel");
}; {
  auto c0 = makeptr(channel, int);
  auto c1 = new nbchannel<int>;
  test(typeid(c0).name() == typeid(c1).name());
  pass("makeptr macro for nbchannel");
}
  return 0;
}