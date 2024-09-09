#include "iostream"

#define test(condition) { if (!(condition)) { std::printf("Test [%s] failed.\n", #condition); exit(1);} }
#define pass(name) std::printf("Test [%s] passed.\n", name)