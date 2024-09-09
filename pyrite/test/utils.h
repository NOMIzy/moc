#include "iostream"

#define test(condition)        \
	{                            \
		if (!(condition)) exit(1); \
	}
#define pass(name) std::printf("Test [%s] passed.\n", name)