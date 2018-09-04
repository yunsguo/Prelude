/*
* function.cpp
* testing file for function.h
* no actual implenmetaion included
* Yunsheng Guo yguo125@syr.edu
*/

#pragma once
#ifdef FUNCTION_CPP

#include "function.h"
#include <iostream>

using namespace fcl;

int test_function1(nontrivial a, nontrivial b, nontrivial c, nontrivial d)
{
	int a1 = a;
	int b1 = b;
	int c1 = c;
	int d1 = d;
	return (a1 + b1)*c1 - d1;
}

int test_function2(int a, int b, int c, int d)
{
	return (a + b)*c - d;
}

NA test_function3(int i,char c, float f, double d)
{
	std::cout << i << c << f << d << std::endl;
	return NA();
}

int main()
{
	details::func_container<0, 0, int, nontrivial, nontrivial, nontrivial, nontrivial> f1(test_function1, std::make_tuple(), std::make_tuple());
	std::cout << f1.invoke(std::make_tuple<nontrivial, nontrivial, nontrivial, nontrivial>(4, 3, 2, 1)) << std::endl;

	std::cout << std::endl << "function front operation start" << std::endl << std::endl;
	auto f2 = f1.push_front(4);
	auto f3 = f2->push_front(3);
	auto f4 = f3->push_front(2);
	auto r = f4->push_front(1);

	std::cout << r << std::endl;
	std::cout << std::endl << "function front operation end" << std::endl << std::endl;

	std::cout << std::endl << "function back operation start" << std::endl << std::endl;
	auto f21 = f1.push_back(1);
	auto f31 = f21->push_back(2);
	auto f41 = f31->push_back(3);
	auto r1 = f41->push_back(4);

	std::cout << r1 << std::endl;
	std::cout << std::endl << "function back operation end" << std::endl << std::endl;

	//Function<int, int, int, int, int> tf1(test_function2);

	Function<int, int, int, int, int> tf1 = test_function2;

	Function<int, nontrivial, nontrivial, nontrivial, nontrivial> tf2(test_function1);

	std::cout << tf1(4, 3, 2, 1) << std::endl;

	std::cout << (tf1 << 4 << 3 << 2 << 1) << std::endl;

	std::cout << (4 >>= 3 >>= 2 >>= 1 >>= tf1) << std::endl;

	std::cout << std::endl << "function reverse operation start" << std::endl << std::endl;

	std::cout << (nontrivial(4) >>= nontrivial(3) >>= nontrivial(2) >>= nontrivial(1) >>= tf1) << std::endl;

	std::cout << std::endl << "function reverse operation end" << std::endl << std::endl;

	Function<int, int, int, int, int> tf11(tf1);

	std::cout << std::endl << "f copy: " << tf11(4, 3, 2, 1) << std::endl << std::endl;

	Function<int, int, int, int, int> tf12 = tf1;

	std::cout << std::endl << "f copy2: " << tf12(4, 3, 2, 1) << std::endl << std::endl;

	Function<int, int, int, int, int> tf13(std::move(Function<int, int, int, int, int>(test_function2)));

	std::cout << std::endl << "f move: " << tf13(4, 3, 2, 1) << std::endl << std::endl;

	Function<int, int, int, int, int> tf14 = Function<int, int, int, int, int>(test_function2);

	std::cout << std::endl << "f construct assign : " << tf14(4, 3, 2, 1) << std::endl << std::endl;

	Function<int, int, int, int, int>* fptr = new Function<int, int, int, int, int>(tf14);

	delete fptr;

	std::cout << util::type<Function<int, int, int, int, int>>::infer() << std::endl;

	Function<NA, int, char, float, double> tf15(test_function3);

	tf15 << 1 << '2' << 3.0f << 4.0;

	4 >>= '3' >>= 2.0f >>= 1.0 >>= tf15;
}

#endif