#include "../src/package.h"

#include "utils.h"

int main() {
	prt::package p;
	p.identifier = "test identifier";
	p.sequence = 2333;
	p.body = prt::bytes("test body");

	prt::package p1;
	p1.identifier = "test identifier";
	p1.sequence = 2333;
	p1.body = prt::bytes("test body");

	test(p == p1);
	p1.sequence = 2334;
	test(!(p == p1));
	pass("bool package::operator==");

	test(p != p1);
	p1.sequence = 2333;
	test(!(p != p1));
	pass("bool package::operator!=");

	prt::package p2(p.to_bytes());
	test(p == p2);
	pass("package convertion");

	return 0;
}