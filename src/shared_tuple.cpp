#pragma once

#ifdef SHARED_TUPLE_CPP

#include "shared_tuple.h"
#include "meta.h"
#include <iostream>

using namespace fcl;

void display(const shared_tuple<nontrivial, nontrivial, nontrivial, nontrivial>& st)
{
	std::tuple<nontrivial, nontrivial, nontrivial, nontrivial> tp = st.to_tuple();
	std::cout
		<< std::endl
		<< "shared_tuple: "
		<< "(" << (int)std::get<0>(tp) << ","
		<< (int)std::get<1>(tp) << ","
		<< (int)std::get<2>(tp) << ","
		<< (int)std::get<3>(tp) << ")"
		<< std::endl;
}

void display(const shared_tuple<int, int, int, int>& st)
{
	std::tuple<int, int, int, int> tp = st.to_tuple();
	std::cout
		<< std::endl
		<< "shared_tuple: "
		<< "(" << std::get<0>(tp) << ","
		<< std::get<1>(tp) << ","
		<< std::get<2>(tp) << ","
		<< std::get<3>(tp) << ")"
		<< std::endl;
}

int main()
{
	simple_shared<nontrivial> sss(nontrivial(5));
	std::cout << "non trivial: " << (int)sss.value() << std::endl;
	std::cout << std::endl << "shared_tuple operation start" << std::endl << std::endl;
	shared_tuple<nontrivial> st1(nontrivial(4));
	auto st2 = st1.push_back(nontrivial(3));
	auto st3 = st2.push_back(nontrivial(2));
	auto st4 = st3.push_back(nontrivial(1));
	shared_tuple<int> st11(4);
	auto st21 = st11.push_front(3);
	auto st31 = st21.push_front(2);
	auto st41 = st31.push_front(1);
	std::cout <<std::endl<< "shared_tuple push end" << std::endl << std::endl;
	display(st4);
	display(st41);
}
#endif