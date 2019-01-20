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

template<typename a>
using VT = variant_traits<a>;

int main()
{
	using V1 = variant<int, char, std::string, nontrivial>;
	V1 v;
	v = 5;
	std::cout << "assign v with 5" << std::endl;
	std::cout << "v is int: " << VT<V1>::is_of<int>(v) << std::endl;
	std::cout << "v is char: " << VT<V1>::is_of<char>(v) << std::endl;
	std::cout << "get v as int: " << VT<V1>::get<int>(v) << std::endl;
	v = 'c';
	std::cout << "assign v with \'c\'" << std::endl;
	std::cout << "v is int: " << VT<V1>::is_of<int>(v) << std::endl;
	std::cout << "v is char: " << VT<V1>::is_of<char>(v) << std::endl;
	std::cout << "get v as char: " << VT<V1>::get<char>(v) << std::endl;
	v = nontrivial(7);
	std::cout << "assign v with nontrivial 7" << std::endl;
	std::cout << "v is int: " << VT<V1>::is_of<int>(v) << std::endl;
	std::cout << "v is nontrivial: " << VT<V1>::is_of<nontrivial>(v) << std::endl;
	std::cout << "get v as nontrivial: " << VT<V1>::get<nontrivial>(v) << std::endl;

	function<int, int> f(inc);
	std::cout << "function test: " << f(5) << std::endl;
	using V2 = variant<Nothing, function<int, int>>;
	V2 mf;
	mf = f;
	std::cout << "assign mf with inc" << std::endl;
	std::cout << "mf is Nothing: " << VT<V2>::is_of<Nothing>(mf) << std::endl;
	std::cout << "mf is function<int, int>: " << VT<V2>::is_of<function<int, int>>(mf) << std::endl;
	std::cout << "get v as a function and call with 5: " << VT<V2>::get<function<int, int>>(mf)(5) << std::endl;

	std::cout << "inferred type V1: " << util::type<V1>::infer() << std::endl;
	std::cout << "inferred type V2: " << util::type<V2>::infer() << std::endl;

}
#endif