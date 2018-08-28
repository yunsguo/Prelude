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

int add(int a, nontrivial&& b) { return a + (int)b; }

int main()
{
	nontrivial test(5);
	auto test1 = test;
	nontrivial test3(std::move(nontrivial(3)));

	std::cout << util::type<TMP::List<int, char, bool, float>>::infer() << std::endl;

	std::cout  << util::type<std::tuple<int, wchar_t, bool, nullptr_t>>::infer() << std::endl;
}

#endif