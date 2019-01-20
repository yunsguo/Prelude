/*
* meta.cpp
* test stub for meta.h
* no actual implenmetaion included
* Yunsheng Guo yguo125@syr.edu
*/

#pragma once
#ifdef META_CPP

#include "meta.h"
#include <iostream>

using namespace fcl;

void do1(nontrivial&& nt) { nontrivial test(std::forward<nontrivial>(nt)); }
void do2(nontrivial& nt) { nt = nt + 1; }
void do3(nontrivial nt) { nt = nt + 1; }

template<typename f>
void test_with(f func, std::string str)
{
	std::cout << std::endl << "call " << str << ": " << std::endl;
	func();
	std::cout << "end call." << std::endl << std::endl;
}

int main()
{
	test_with([]() {nontrivial test; }, "default constructor");

	test_with([]() {nontrivial test(1);; }, "conversion constructor");

	test_with([]() {nontrivial copy(1); auto test(copy); }, "copy constructor");

	test_with([]() {nontrivial test(std::move(nontrivial(2))); }, "move constructor");

	nontrivial test1(1);

	nontrivial test2(2);

	nontrivial test3(3);

	nontrivial test4(4);

	test_with([&]() {do1(std::move(test1)); }, "by rvalue ref");
	std::cout << test1 << std::endl;

	test_with([&]() {do2(test2); }, "by ref");
	std::cout << test2 << std::endl;

	test_with([&]() 
	{
		auto test21 = test3;
		do2(test21); 
	}, "by ref passing value");
	std::cout << test3 << std::endl;

	test_with([=]() {do3(test3); }, "by value");
	std::cout << test3 << std::endl;


	test_with([&]() {do3(test3); }, "by value passing ref");
	std::cout << test3 << std::endl;


	test_with([&]() {do3(std::move(test4)); }, "by value passing rvalue ref");
	std::cout << test4 << std::endl;


	test_with([&]() {do3(nontrivial(4)); }, "by value passing implicit rvalue");

	std::cout << std::endl << "util::type::infer() test: " << std::endl;

	std::cout << util::type<TMP::list<int, char, bool, float>>::infer() << std::endl;

	std::cout << util::type<std::tuple<int, wchar_t, bool, nullptr_t>>::infer() << std::endl;
}

#endif