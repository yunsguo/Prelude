#ifdef TMP_CPP

#include "TMP.h"
#include "util_type.h"
#include <iostream>

using namespace TMP;

template<typename T>
void display(std::string msg="") { std::cout << msg << util::type<T>::infer() << std::endl; }
template<typename T>
void show(std::string msg = "") { std::cout << msg << util::show<T>() << std::endl; }

int main()
{
	using ls1 = List<int, int, int, int, int>;
	using ls2 = List<double, double, double, double, double>;
	using ls3 = List<int, double, float, char>;
	using pk1 = Pack<int, int, int, int, int>;
	using pk2 = Pack<double, double, double, double, double>;
	using pk3 = Pack<int, double, float, char>;
	using tu1 = std::tuple<int, int, int, int, int>;
	using tu2 = std::tuple<double, double, double, double, double>;
	using tu3 = std::tuple<int, double, float, char>;

	display<ls1>("list 1: ");
	display<ls2>("list 2: ");

	display<concat<ls1, ls2>::type>("concat list 1 & list 2: ");

	display<typename to_tuple<ls3>::type>("list 3 to tuple: ");

	display<head<ls3>::type>("head of list3: ");
	display<head<pk3>::type>("head of pack3: ");
	display<head<tu3>::type>("head of tuple3: ");

	display<last<ls3>::type>("last of list3: ");
	display<last<pk3>::type>("last of pack3: ");
	display<last<tu3>::type>("last of tuple3: ");

	display<tail<ls3>::type>("tail of list3: ");
	display<tail<pk3>::type>("tail of pack3: ");
	display<tail<tu3>::type>("tail of tuple3: ");
	show<fmap<tail,pk3>::type>("fmap tail of pack3: ");
	display<fmap<tail, tu3>::type>("fmap tail of tuple3: ");

	display<init<ls3>::type>("init of list3: ");
	display<init<pk3>::type>("init of pack3: ");
	display<init<tu3>::type>("init of tuple3: ");

	display<take<ls3, 2>::type>("list3 take 2: ");
	display<take<ls3, 11>::type>("list3 take 11: ");
	display<take<ls3, 0>::type>("list3 take 0: ");
	display<take<pk3, 2>::type>("pack3 take 2: ");
	display<take<tu3, 2>::type>("tuple3 take 2: ");

	display<drop<ls3, 2>::type>("list3 drop 2: ");
	display<drop<ls3, 11>::type>("list3 drop 11: ");
	display<drop<ls3, 0>::type>("list3 drop 0: ");
	display<drop<pk3, 2>::type>("pack3 drop 2: ");
	display<drop<tu3, 2>::type>("tuple3 drop 2: ");

	display<reverse_t<ls3>>("reverse ls3: ");
	display<reverse_t<pk3>>("reverse pk3: ");
	display<reverse_t<tu3>>("reverse tu3: ");

	display<flip_type<ls3>>("reverse ls3: ");
	display<flip_type<pk3>>("reverse pk3: ");
	display<flip_type<tu3>>("reverse tu3: ");

	using lst4 = TMP::List<size_t>;
	using lst5 = TMP::List<int, char>;
	using lst6 = TMP::List<int, size_t, char, double, float>;
	auto tuple3 = std::make_tuple(1, 2.0, 3.0f, '4');
	std::tuple<char,float,double,int> tuple3r = details::reverse(std::move(tuple3));
	std::cout << "infer list4 type: " << util::type<lst4>::infer() << std::endl
		<< "lst6 elem_index float: " << elem_index<float, lst6>::value << std::endl
		<< "lst6 index 2: " << util::type<index<lst6, 2>::type>::infer() << std::endl
		<< "char elem lst5: " << elem<lst5, char>::value << std::endl
		<< "float elem lst5: " << elem<lst5, float>::value << std::endl
		<< "infer tuple3 type: " << util::type<tu3>::infer() << std::endl
		<< "infer reverse tuple 3 type: " << util::type<decltype(details::reverse(std::move(tuple3)))>::infer() << std::endl
		<< "("<<std::get<0>(tuple3r)<<", " << std::get<1>(tuple3r) << ", " << std::get<2>(tuple3r) << ", " << std::get<3>(tuple3r) << ")" << std::endl;

}
#endif