#include "clock.h"

#include <chrono>
#include <cstdlib>
#include <thread>

i64 moc::get_tick() {
  auto now = std::chrono::system_clock::now().time_since_epoch();
  return std::chrono::duration_cast<std::chrono::duration<std::int64_t, std::milli>>(now).count();
}

moc::clock::clock() {
  last = moc::get_tick();
}

i64 moc::clock::tick(i64 period_time) {
  auto now = moc::get_tick();
  auto used = now - last;
  auto should_wait = period_time - used;
  if (should_wait > 0)
    std::this_thread::sleep_for(std::chrono::milliseconds(should_wait));
  last = moc::get_tick();
  return used;
}
