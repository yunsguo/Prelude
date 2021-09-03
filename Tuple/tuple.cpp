#include <iostream>
#include <cassert>

#include "tuple.h"
#include "non_trivial.h"

using namespace fcl;

void display(const tuple<non_trivial, non_trivial, non_trivial, non_trivial> &st)
{
	std::cout
		<< std::endl
		<< "tuple: "
		<< "(" << (int)std::get<0>(st) << ","
		<< (int)std::get<1>(st) << ","
		<< (int)std::get<2>(st) << ","
		<< (int)std::get<3>(st) << ")"
		<< std::endl;
}

void display(const tuple<int, int, int, int> &st)
{
	std::cout
		<< std::endl
		<< "tuple: "
		<< "(" << std::get<0>(st) << ","
		<< std::get<1>(st) << ","
		<< std::get<2>(st) << ","
		<< std::get<3>(st) << ")"
		<< std::endl;
}

int main()
{
	std::cout
		<< "start operation. " << std::endl;
	if (!std::is_same<std::tuple<>, tuple<>>::value)
		std::cout << "not ";

	std::cout << "using std::tuple." << std::endl;
	std::cout << std::endl
			  << "operation starts:" << std::endl
			  << std::endl;
	auto st1 = fcl::make_tuple<non_trivial>(non_trivial(4));
	auto st22 = fcl::make_tuple<non_trivial>(non_trivial(3));
	auto st2 = std::tuple_cat(std::move(st1), std::move(st22));
	auto st3 = std::tuple_cat(std::move(st2), fcl::make_tuple<non_trivial>(non_trivial(2)));
	auto st4 = std::tuple_cat(std::move(st3), fcl::make_tuple<non_trivial>(non_trivial(1)));
	auto st4c = st4;
	tuple<int> st11 = fcl::make_tuple<int>(4);
	auto st21 = std::tuple_cat(fcl::make_tuple<int>(3), std::move(st11));
	auto st31 = std::tuple_cat(fcl::make_tuple<int>(2), std::move(st21));
	auto st41 = std::tuple_cat(fcl::make_tuple<int>(1), std::move(st31));
	auto st5 = fcl::make_tuple<int, int, int, int>(1, 2, 3, 4);
	std::cout << std::endl
			  << "operation ends." << std::endl
			  << std::endl;
	display(st4);
	display(st41);
	display(st4c);
	display(st5);
	tuple<int, int, int> t = {1, 2, 3};
	tuple<int> a;
	tuple<int, int> b;
	std::tie(b, a) = split_at<2>(t);
	assert(std::get<0>(a) == 3);
	assert(std::get<0>(b) == 1 && std::get<1>(b) == 2);
	std::cout
		<< "end operation. " << std::endl;
}