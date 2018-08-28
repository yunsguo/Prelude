/*
*   TMP.h:
*   a template meta programming package in Haskell style for general use
*   Language: C++, Visual Studio 2017
*   Platform: Windows 10 Pro
*   Application: recreational
*   Author: Yunsheng Guo, yguo125@syr.edu
*/


/*
*
*   Package Operations:
*	a lot of type list operations
*	converting freely between std::tuple and TMP::Pack and TMP::Cons
*	has a list_trait struct for automatic conversion from any specilized type to TMP::Cons
*	naming convensions follows Haskell, capitals are types and all lower cases are meta functions
*
*   Public Interface:
*	technically none.
*
*   Build Process:
*
*   Maintenance History:
*   June 6
*   it all starts from here
*	August 25
*	final review for publish, has not changed much due to the bed rock position in dependency hierarchy
*
*
*/

#pragma once
#ifndef _TMP_
#define _TMP_

#include <tuple>


namespace TMP
{
	struct Nil {};

	//type representing a type list
	template<typename a, typename b>
	struct Cons {};

	//type container
	template<typename ... as>
	struct Pack {};

	//meta function for constructing a list 
	template<typename a, typename b>
	struct cons;

	template<typename a, typename b>
	using cons_t = typename cons<a, b>::type;

	template<typename a, typename b, typename c>
	struct cons<a, Cons<b, c>> { using type = Cons<a, Cons<b, c>>; };

	template<typename a>
	struct cons<a, Nil> { using type = Cons<a, Nil>; };

	template<typename a, typename ...as>
	struct cons<Pack<a>, Pack<as...>> { using type = Pack<a, as...>; };

	template<typename a, typename ...as>
	struct cons<std::tuple<a>, std::tuple<as...>> { using type = std::tuple<a, as...>; };

	//meta function converting other type to type list
	template<typename T>
	struct to_list;

	//informal type list  constructor 
	template<typename ...as>
	using List = typename to_list<Pack<as...>>::type;

	template<>
	struct to_list<Pack<>> { using type = Nil; };

	template<typename T, typename...Ts>
	struct to_list<Pack<T, Ts...>> { using type = Cons<T, typename to_list<Pack<Ts...>>::type>; };

	template<typename ...Ts>
	struct to_list<std::tuple<Ts...>> { using type = List<Ts...>; };

	//list trait for automatic conversion 
	//such that other type with list trait can be used as list 
	template<typename a, typename = void>
	struct list_trait;

	template<typename a, typename b = void>
	using list_trait_t = typename list_trait<a, b>::type;

	template<>
	struct list_trait<Nil> { using type = Nil; };

	template<typename a, typename b>
	struct list_trait<Cons<a, b>> { using type = Cons<a, b>; };

	template<typename ...as>
	struct list_trait<Pack<as...>> { using type = List<as...>; };

	template<typename ...as>
	struct list_trait<std::tuple<as...>> { using type = List<as...>; };

	//meta function converting other type to type pack
	template<typename a>
	struct to_pack { using type = typename to_pack<list_trait_t<a>>::type; };

	template<typename a>
	using to_pack_t = typename to_pack<a>::type;

	template<>
	struct to_pack<Nil> { using type = Pack<>; };

	template<typename a, typename b>
	struct to_pack<Cons<a, b>> { using type = typename cons<Pack<a>, typename to_pack<b>::type>::type; };

	template<typename ... as>
	struct to_pack<std::tuple<as...>> { using type = Pack<as...>; };

	//meta function converting other type to tuple
	template<typename a>
	struct to_tuple;

	template<>
	struct to_tuple<Nil> { using type = std::tuple<>; };

	template<typename a, typename b>
	struct to_tuple<Cons<a, b>> { using type = typename cons<std::tuple<a>, typename to_tuple<b>::type>::type; };

	template<typename ... as>
	struct to_tuple<Pack<as...>> { using type = std::tuple<as...>; };

	//meta function concatenating two list
	template<typename a, typename b>
	struct concat { using type = typename concat<list_trait_t<a>, list_trait_t<b>>::type; };

	template<typename a, typename b>
	using concat_t = typename concat<a, b>::type;

	template<typename a>
	struct concat<Nil, a> { using type = a; };

	template<typename a>
	struct concat<a, Nil> { using type = a; };

	template<typename a, typename b, typename c>
	struct concat<Cons<a, b>, c> { using type = Cons<a, concat_t<b, c>>; };

	//function mapping for easier implenmentation
	template<template<typename> typename l, typename fa>
	struct fmap;

	template<template<typename> typename l, typename fa>
	using fmap_type = typename fmap<l, fa>::type;

	template<template<typename> typename l, typename ...as>
	struct fmap<l, Pack<as...>> { using type = typename to_pack<typename l<List<as...>>::type>::type; };

	template<template<typename> typename l, typename ...as>
	struct fmap<l, std::tuple<as...>> { using type = typename to_tuple<typename l<List<as...>>::type>::type; };

	//Extract the first component of a pair
	template<typename a>
	struct fst;

	template<typename a, typename b>
	struct fst<Pack<a, b>> { using type = a; };

	//Extract the second component of a pair
	template<typename a>
	struct snd;

	template<typename a, typename b>
	struct snd<Pack<a, b>> { using type = b; };

	//Extract the first element of a list, which must be non-empty.
	template<typename a>
	struct head { using type = typename head<typename list_trait<a>::type>::type; };

	template<typename a>
	using head_t = typename head<a>::type;

	template<typename a, typename b>
	struct head<Cons<a, b>> { using type = a; };

	template<typename a, typename ...as>
	struct head<std::tuple<a, as...>> { using type = a; };

	template<typename a, typename ...as>
	struct head<Pack<a, as...>> { using type = a; };

	//Extract the last element of a list, which must be finite and non-empty.
	template<typename a>
	struct last { using type = typename last<typename list_trait<a>::type>::type; };

	template<typename a>
	using last_t = typename last<a>::type;

	template<typename a, typename b>
	struct last<Cons<a, b>> { using type = typename last<b>::type; };

	template<typename a>
	struct last<Cons<a, Nil>> { using type = a; };

	//Extract the elements after the head of a list, which must be non-empty.
	template<typename a>
	struct tail { using type = typename tail<typename list_trait<a>::type>::type; };

	template<typename a, typename b>
	struct tail<Cons<a, b>> { using type = b; };

	//Return all the elements of a list except the last one. The list must be non-empty.
	template<typename a>
	struct init { using type = typename init<typename list_trait<a>::type>::type; };

	template<typename a>
	using init_t = typename init<a>::type;

	template<typename a, typename b>
	struct init<Cons<a, b>> { using type = Cons<a, typename init<b>::type>; };

	template<typename a, typename b>
	struct init<Cons<a, Cons<b, Nil>>> { using type = Cons<a, Nil>; };

	template<typename a>
	struct init<Cons<a, Nil>> { using type = Nil; };

	// returns the prefix of xs of length n, or xs itself if n > length xs
	template<typename a, size_t N>
	struct take { using type = typename take<typename list_trait<a>::type, N>::type; };

	template<size_t N>
	struct take<Nil, N> { using type = Nil; };

	template<typename a, typename b, size_t N>
	struct take<Cons<a, b>, N> { using type = Cons<a, typename take<b, N - 1>::type>; };

	template<typename a, typename b>
	struct take<Cons<a, b>, 0> { using type = Nil; };

	//returns the suffix of xs after the first n elements, or [] if n > length xs
	template<typename a, size_t N>
	struct drop { using type = typename drop<typename list_trait<a>::type, N>::type; };

	template<size_t N>
	struct drop<Nil, N> { using type = Nil; };

	template<typename a, typename b, size_t N>
	struct drop<Cons<a, b>, N> { using type = typename drop<b, N - 1>::type; };

	template<typename a, typename b>
	struct drop<Cons<a, b>, 0> { using type = Cons<a, b>; };

	//Returns the size/length of a finite structure as an Int.
	template<typename a>
	struct length { enum { value = length<typename list_trait<a>::type>::value }; };

	template<>
	struct length<Nil> { enum { value = 0 }; };

	template<typename a, typename b>
	struct length<Cons<a, b>> { enum { value = 1 + length<b>::value }; };

	template<typename ...as>
	struct length<Pack<as...>> { enum { value = sizeof...(as) }; };

	template<typename ...as>
	struct length<std::tuple<as...>> { enum { value = sizeof...(as) }; };

	//list index meta function
	template<typename a, typename b>
	struct elem_index;

	template<typename a, typename b>
	struct elem_index<a, Cons<a, b>> { enum { value = 0 }; };

	template<typename a, typename b, typename c>
	struct elem_index<a, Cons<b, c>> { enum { value = 1 + elem_index<a, c>::value }; };

	//list indexing meta function
	template<typename a, size_t N>
	struct index { using type = typename index<typename list_trait<a>::type,N>::type; };

	template<size_t N>
	struct index<Nil, N> {};

	template<typename a, typename b>
	struct index<Cons<a, b>, 0> { using type = a; };

	template<typename a, typename b, size_t N>
	struct index<Cons<a, b>, N> { using type = typename index<b, N - 1>::type; };

	//Does the element occur in the structure?
	template<typename a, typename b>
	struct elem :public std::bool_constant<elem<typename list_trait<a>::type, b>::value> {};

	template<typename a>
	struct elem<Nil, a> :public std::false_type {};

	template<typename a, typename b>
	struct elem<Cons<a, b>, a> :public std::true_type {};

	template<typename a, typename b, typename c>
	struct elem<Cons<a, b>, c> :public std::bool_constant<elem<b, c>::value> {};

}

namespace details
{
	template<typename a, typename b = TMP::Nil>
	struct rev { using type = typename rev<TMP::list_trait_t<a>, TMP::list_trait_t<b>>::type; };

	template<typename a>
	struct rev<TMP::Nil, a> { using type = a; };

	template<typename x, typename xs, typename a>
	struct rev<TMP::Cons<x, xs>, a> { using type = typename rev<xs, TMP::Cons<x, a>>::type; };
}

namespace TMP
{
	//reverse a returns the elements of a in reverse order.
	template<typename a>
	struct reverse { using type = typename details::rev<list_trait_t<a>>::type; };

	template<typename a>
	using reverse_t = typename details::rev<list_trait_t<a>>::type;

}

#endif