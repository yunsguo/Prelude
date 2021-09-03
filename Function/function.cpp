/*
* function.cpp
* testing file for function.h
* no actual implenmetaion included
* Yunsheng Guo yguo125@syr.edu
*/

#include <iostream>
#include <cassert>
#include <utility>

#include "function.h"
#include "non_trivial.h"

using namespace fcl;

struct test
{
	static int member_function(int a, int b) { return a + b; }
};

int test_function1(non_trivial a, non_trivial b, non_trivial c, non_trivial d)
{
	int a1 = a;
	int b1 = b;
	int c1 = c;
	int d1 = d;
	return (a1 * 2 + b1) * c1 - d1;
}
int test_function1p(non_trivial &a, non_trivial b, non_trivial c, non_trivial d)
{
	int a1 = a;
	int b1 = b;
	int c1 = c;
	int d1 = d;
	return (a1 * 2 + b1) * c1 - d1;
}
int test_function1pp(non_trivial a, non_trivial b, non_trivial c, non_trivial &d)
{
	int a1 = a;
	int b1 = b;
	int c1 = c;
	int d1 = d;
	return (a1 * 2 + b1) * c1 - d1;
}
int test_function1ppp(non_trivial &a, non_trivial b, non_trivial c, non_trivial &d)
{
	int a1 = a;
	int b1 = b;
	int c1 = c;
	int d1 = d;
	return (a1 * 2 + b1) * c1 - d1;
}

int test_function2(int a, int b, int c, int d)
{
	return (a * 2 + b) * c - d;
}

template <typename P>
int test_function3(P p, int a, int b)
{
	return p(a) * b;
}

int main()
{
	std::cout << "start operation. " << std::endl;
	{
		using F = decltype(test_function1);
		auto f1 = test_function1 < non_trivial(4);
		assert(f1(non_trivial(3), non_trivial(2), non_trivial(1)) == 21);
		auto f2 = f1 < non_trivial(3);
		assert(f2(non_trivial(2), non_trivial(1)) == 21);
		auto f3 = f2 < non_trivial(2);
		assert(f3(non_trivial(1)) == 21);
		assert((f3 < non_trivial(1)) == 21);
	}
	{
		auto nt = non_trivial(4);
		auto f1 = test_function1p < nt;
		assert(f1(non_trivial(3), non_trivial(2), non_trivial(1)) == 21);
		auto f2 = f1 < non_trivial(3);
		assert(f2(non_trivial(2), non_trivial(1)) == 21);
		auto f3 = f2 < non_trivial(2);
		assert(f3(non_trivial(1)) == 21);
		assert((f3 < non_trivial(1)) == 21);
	}
	assert(boxed(test_function2) < 4 < 3 < 2 < 1 == 21);

	assert(boxed(test::member_function) < 1 < 2 == 3);
	assert((test_function3<partial_applied_t<decltype(test::member_function)>> * boxed(test::member_function))(1, 2, 3) == 9);
	assert((test_function3<partial_applied_t<decltype(test::member_function)>> * boxed(test::member_function)) < 1 < 2 < 3 == 9);
	std::cout << "end operation. " << std::endl;
}
