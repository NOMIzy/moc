#include "log.h"
#include <stdarg.h>

namespace moc {
void log(std::string msg) {
  std::printf("\033[32m[I] %s\n\033[0m", msg.c_str());
}

void warn(std::string msg) {
  std::printf("\033[33m[W] %s\n\033[0m", msg.c_str());
}

void panic(std::string msg) {
  std::printf("\033[31m[E] %s\n\033[0m", msg.c_str());
  exit(1);
}

void logf(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  std::printf("\033[32m[I] ");
  std::vprintf(fmt, args);
  std::printf("\033[0m");
  va_end(args);
}

void warnf(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  std::printf("\033[33m[W] ");
  std::vprintf(fmt, args);
  std::printf("\033[0m");
  va_end(args);
}

void panicf(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  std::printf("\033[31m[E] ");
  std::vprintf(fmt, args);
  std::printf("\033[0m");
  va_end(args);
  exit(1);
}
}