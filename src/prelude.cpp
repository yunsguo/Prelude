/*
* prelude.cpp
* test stub for prelude.h
* no actual implenmetaion included
* Yunsheng Guo yguo125@syr.edu
*/

#ifdef PRELUDE_CPP

#include "prelude.h"
#include <iostream>

using namespace fcl;

struct test_struct
{
	test_struct() {}
	test_struct(size_t i) :index(i) {}

	size_t value()const { return index; }

private:
	size_t index;
};

template<>
struct Ord<test_struct>
{
	using pertain = std::true_type;
	static Ordering compare(const test_struct& one, const test_struct& other)
	{
		if (one.value() > other.value()) return Ordering::GT;
		if (one.value() == other.value()) return Ordering::EQ;
		return Ordering::LT;
	}
};

template<>
struct Show<test_struct>
{
	using pertain = std::true_type;
	static std::string show(const test_struct& value) { return std::to_string(value.value()); }
};

int increment(int a) { return a + 1; }

int decrement(int a) { return a - 1; }

bool is_nothing(Maybe<int> a)
{
	return of<bool>::pattern<Maybe<int>>(a)
		.match<Nothing>(true)
		.match<Just>(false);
}

Maybe<int> maybe_add(Maybe<int> a, Maybe<int> b)
{
	return of<Maybe<int>>::pattern<Maybe<int>, Maybe<int>>(a, b)
		.match<Nothing, Nothing>(Nothing())
		.match<Just<int>, Nothing>(Nothing())
		.match<Nothing, Just<int>>(Nothing())
		.match<Just<int>, Just<int>>(
			function<Maybe<int>, Just<int>, Just<int>>
			([](Just<int> a, Just<int> b)->Maybe<int>
	{
		return Maybe<int>(a.value + b.value);
	}));
}

int main()
{
	test_struct ts1(1);
	test_struct ts2(2);
	std::cout << "test_struct is Eq: " << Eq<test_struct>::pertain::value << std::endl;
	std::cout << "ts1 == ts2: " << (ts1 == ts2) << std::endl;
	std::cout << "ts1 >= ts2: " << (ts1 >= ts2) << std::endl;
	std::cout << "ts1 < ts2: " << (ts1 < ts2) << std::endl;
	std::cout << "show ts1: " << ts1 << std::endl;

	function<int, int> f(increment);
	Maybe<int> m(5);
	Maybe<int> m1;
	Maybe<int> m3(7);
	Maybe<function<int, int>> mf(increment);
	Maybe<function<int, int>> m2 = function<int,int>(increment);
	std::cout << "Maybe Int: " << Maybe<int>(3) << std::endl;
	std::cout << "Nothing < Just 5: " << (m1 < m) << std::endl;
	std::cout << "Nothing || Just 5: " << (Maybe<int>() || Maybe<int>(5)) << std::endl;
	std::cout << "Just increment << Nothing: " << (Maybe<function<int, int>>(increment) <<= Maybe<int>()) << std::endl;
	std::cout << "Just increment << Just 5: " << (Maybe<function<int, int>>(increment) <<= Maybe<int>(5)) << std::endl;
	std::cout << "Nothing >>= Just increment: " << (Maybe<int>() >>= Maybe<function<int, int>>(increment)) << std::endl;
	std::cout << "Just 5 >>= Just increment: " << (Maybe<int>(5) >>= Maybe<function<int, int>>(increment)) << std::endl;
	auto t3 = std::make_tuple<int, int, int>(1, 2, 3);
	std::cout << "t3: " << t3 << std::endl;
	Maybe<pair<int, std::string>> r = std::make_pair<int, std::string>(1, "content");
	std::cout << "reaction: " << r << std::endl;
	list<int> list = { 2,3,5,7,11,13,17,19 };
	std::cout << "list: " << list << std::endl;
	std::cout << "is_nothing Nothing: " << is_nothing(m1) << std::endl;
	std::cout << "is_nothing Just 5: " << is_nothing(m) << std::endl;
	std::cout << "maybe_add Nothing Just 5: " << maybe_add(m1, m) << std::endl;
	std::cout << "maybe_add Just 7 Just 5: " << maybe_add(m3, m) << std::endl;
	data<int, float, char, std::string> tv1 = 1;
	std::cout << "variant int: " << tv1 << std::endl;
	tv1 = 2.0f;
	std::cout << "variant float: " << tv1 << std::endl;
	tv1 = 'c';
	std::cout << "variant char: " << tv1 << std::endl;
	tv1 = std::string("the 4th type");
	std::cout << "variant string: " << tv1 << std::endl;
}

#endif