/*
* variant.cpp
* test stub for variant.h
* no actual implenmetaion included
* Yunsheng Guo yguo125@syr.edu
*/

#ifdef VARIANT_CPP

#include "variant.h"
#include <iostream>
#include <string>
#include "function.h"

using namespace fcl;

int inc(int a) { return a + 1; }

using Nothing = std::tuple<>;

int main()
{
	variant<int, char, std::string, nontrivial> v;
	v = 5;
	std::cout << v.is_of<int>() << std::endl;
	std::cout << v.is_of<char>() << std::endl;
	std::cout << v.get<int>() << std::endl;
	v = 'c';
	std::cout << v.is_of<int>() << std::endl;
	std::cout << v.is_of<char>() << std::endl;
	std::cout << v.get<char>() << std::endl;
	v = nontrivial(7);
	std::cout << v.is_of<int>() << std::endl;
	std::cout << v.is_of<nontrivial>() << std::endl;
	std::cout << (int)v.get<nontrivial>() << std::endl;

	Function<int, int> f(inc);
	std::cout << "function test: "<<f(5) << std::endl;
	variant<Nothing, Function<int, int>> mf;
	mf = f;
	std::cout << mf.is_of<Nothing>() << std::endl;
	std::cout << mf.is_of<Function<int, int>>() << std::endl;
	std::cout << mf.get<Function<int, int>>()(5) << std::endl;

	std::cout << util::type<variant<Nothing, int>>::infer() << std::endl;

}
#endif