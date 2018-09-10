/*
*   meta.h:
*   forward declaration header plus some miscellaneous type inference stuff
*   Language: C++, Visual Studio 2017
*   Platform: Windows 10 Pro
*   Application: recreational
*   Author: Yunsheng Guo, yguo125@syr.edu
*/


/*
*
*   Package Operations:
*	None, almost all definition will be implenmented or specilized later
*	forward types such that the FP library does not depends on variant or function implenmentation
*
*   Public Interface:
*	None
*
*   Build Process:
*   requires TMP.h
*
*   Maintenance History:
*   June 6
*   contain and separate FP library from TMP.h such that
*	TMP.h can be used for other projects without any problem
*   August 12
*   refactor into a forward declaration header with some useful stuff
*	August 25
*	final review for publish
*
*
*/

#pragma once
#ifndef _META_
#define _META_

#include "TMP.h"
#include <string>
#include <list>

namespace fcl
{
	//corrisponding to () in Haskell, a type meant to be discarded
	using NA = std::tuple<>;

	template<typename a, typename b>
	using Pair = std::pair<a, b>;

	template<typename ...as>
	using Tuple = std::tuple<as...>;

	//generic function container in the form of r function(a first, as... args)
	template<typename r, typename a, typename ...as>
	struct Function;

	//indirect TMF function definition
	template<typename f>
	struct function_traits
	{
		//possess trait or not
		using possess = std::false_type;
		//a Pack with a haskell style ordering
		using type = TMP::Pack<NA, NA>;
		//common partial applied type
		using applied = NA;
		//reversed partial applied type for monad operation
		using monadic_applied = NA;
		//first parameter, last for monadic
		using head = NA;
		//laster parameter, first for monadic
		using last = NA;

		static applied apply(const f&, head&&);

		static monadic_applied monadic_apply(const f&, last&&);

	};

	template<typename f>
	using is_function = typename function_traits<f>::possess;

	template<typename f>
	using function_traits_type = typename function_traits<f>::type;

	template<typename f>
	using applied_type = typename function_traits<f>::applied;

	template<typename f>
	using monadic_applied_type = typename function_traits<f>::monadic_applied;

	template<typename f>
	using head_parameter = typename function_traits<f>::head;

	template<typename f>
	using last_parameter = typename function_traits<f>::last;

	template<typename f, typename = std::enable_if_t<is_function<f>::value>>
	applied_type<f> operator<<(const f& func, head_parameter<f>&& arg)
	{
		return function_traits<f>::apply(func, std::forward<head_parameter<f>>(arg));
	}

	template<typename f, typename = std::enable_if_t<is_function<f>::value>>
	applied_type<f> operator<<(const f& func, const head_parameter<f>& arg)
	{
		auto arg_(arg);
		return function_traits<f>::apply(func, std::move(arg_));
	}

	template<typename f, typename = std::enable_if_t<is_function<f>::value>>
	monadic_applied_type<f> operator>>=(last_parameter<f>&& arg, const f& func)
	{
		return function_traits<f>::monadic_apply(func, std::forward<last_parameter<f>>(arg));
	}

	template<typename f, typename = std::enable_if_t<is_function<f>::value>>
	monadic_applied_type<f> operator>>=(const last_parameter<f>& arg, const f& func)
	{
		auto arg_(arg);
		return function_traits<f>::monadic_apply(func, std::move(arg_));
	}

	//generic or type container forward declaration
	template<typename a, typename b, typename ...rest>
	struct variant;

	//indirect TMF variant definition
	template<typename def>
	struct variant_traits
	{
		//possess trait or not
		using possess = std::false_type;

		//has a default type that can be infered
		//for instance if std::vector<int> is a variant probably has a default type int
		using has_default = std::false_type;

		using default_type = NA;

		//meta function check type could be a variant
		template<typename a>
		using elem = TMP::elem<TMP::List<>, a>;

		//rumtime check containing this type a
		template<typename a>
		static bool is_of(const def&);

		//move the type out
		//throw exception if is not a
		//the variant could be ill-formed after
		template<typename a>
		static a&& move(def&);

		//copy the type it contains
		//throw exception if is not a
		template<typename a>
		static const a& get(const def&);
	};

	//Haskell typeclass declarations

	template<typename a>
	struct Eq;

	template<typename a>
	struct Ord;

	enum Ordering { LT, EQ, GT };

	template<typename a>
	struct Show;

	template<typename a>
	struct Monoid;

	template<template<typename> typename F>
	struct Functor;

	template<template<typename> typename I>
	struct Injector;

	template<template<typename> typename A>
	struct Applicative;

	template<template<typename> typename A>
	struct Alternative;

	template<template<typename> typename M>
	struct Monad;

	template<template<typename> typename F>
	struct Foldable;

	template<template<typename> typename T>
	struct Traversable;
}

//give Function and variant TMP list trait for easier readability
namespace TMP
{
	template<typename r, typename a, typename ...as>
	struct list_trait <fcl::Function<r, a, as...>> { using type = List<a, as..., r>; };

	template<typename a, typename b, typename ...rest>
	struct list_trait<fcl::variant<a, b, rest...>> { using type = List<a, b, rest...>; };
}

namespace details
{
	template<char c, typename a>
	struct CharSeparated {};

	template<char c, char d, typename a>
	struct TwoCharSeparated {};

	template<char l, char r, typename a>
	struct BoundBy {};
}

//runtime type inferrence
//if we have a compile-time string oh man this stuff is gonna be wild
namespace util
{
	using TMP::Pack;

	using details::CharSeparated;

	using details::TwoCharSeparated;

	using details::BoundBy;

	//type info infer method 
	template<typename a>
	struct type { static std::string infer() { return typeid(a).name(); } };

	template<char l, char r, typename a>
	struct type<BoundBy<l, r, a>> { static std::string infer() { return l + type<a>::infer() + r; } };

	template<char c>
	struct type<CharSeparated<c, Pack<>>> { static std::string infer() { return ""; } };

	template<char c, typename a>
	struct type<CharSeparated<c, Pack<a>>> { static std::string infer() { return type<a>::infer(); } };

	template<char c, typename a, typename...as>
	struct type<CharSeparated<c, Pack<a, as...>>> { static std::string infer() { return type<a>::infer() + c + type<CharSeparated<c, Pack<as...>>>::infer(); } };

	template<char c, char d>
	struct type<TwoCharSeparated<c, d, Pack<>>> { static std::string infer() { return ""; } };

	template<char c, char d, typename a>
	struct type<TwoCharSeparated<c, d, Pack<a>>> { static std::string infer() { return type<a>::infer(); } };

	template<char c, char d, typename a, typename...as>
	struct type<TwoCharSeparated<c, d, Pack<a, as...>>> { static std::string infer() { return type<a>::infer() + c + d + type<TwoCharSeparated<c, d, Pack<as...>>>::infer(); } };

	template<typename r, typename ...as>
	struct type<fcl::Function<r, as...>> { static std::string infer() { return type<TwoCharSeparated<'-', '>', typename fcl::function_traits<fcl::Function<r, as...>>::type>>::infer(); } };

	template<typename a, typename b, typename ...rest>
	struct type<fcl::variant<a, b, rest...>> { static std::string infer() { return type<CharSeparated<'|', TMP::Pack<a, b, rest...>>>::infer(); } };

	template<typename ...as>
	struct type<std::tuple<as...>> { static std::string infer() { return type<BoundBy<'(', ')', CharSeparated<',', Pack<as...>>>>::infer(); } };

	template<>
	struct type<TMP::Nil> { static std::string infer() { return "[]"; } };

	template<typename a, typename b>
	struct type<TMP::Cons<a, b>> { static std::string infer() { return type<BoundBy<'[', ']', CharSeparated<',', typename TMP::to_pack<TMP::Cons<a, b>>::type>>>::infer(); } };

	//template<>
	//struct type<char> { static std::string infer() { return "Char"; } };

	template<>
	struct type<std::string> { static std::string infer() { return "String"; } };

	//template<>
	//struct type<int> { static std::string infer() { return "Int"; } };

	//template<>
	//struct type<size_t> { static std::string infer() { return "Natural"; } };

	//template<>
	//struct type<float> { static std::string infer() { return "Float"; } };

	//template<>
	//struct type<double> { static std::string infer() { return "Double"; } };
}

//a nontrivial struct for general testing
//put message out on every step of its life cycle
#ifndef _RELEASE_
#include <iostream>
namespace fcl
{
	struct nontrivial
	{
		friend std::ostream& operator<<(std::ostream&, const nontrivial&);

		nontrivial();
		nontrivial(int value);
		nontrivial(const nontrivial& other);
		nontrivial(nontrivial&& other);

		nontrivial& operator=(const nontrivial& other);

		nontrivial& operator=(nontrivial&& other);

		~nontrivial();

		operator int()const;

	private:
		int *ptr_;
	};

	inline nontrivial::nontrivial() :ptr_(nullptr) { std::cout << "default construct... " << std::endl; }

	inline nontrivial::nontrivial(int value) : ptr_(new int(value)) { std::cout << "construct value: " << value << std::endl; }

	inline nontrivial::nontrivial(const nontrivial& other)
	{
		if (other.ptr_ == nullptr)
		{
			std::cout << "copy construct null" << std::endl;
			ptr_ = nullptr;
		}
		else
		{
			std::cout << "copy construct value: " << *other.ptr_ << std::endl;
			ptr_ = new int(*other.ptr_);
		}
	}

	inline nontrivial::nontrivial(nontrivial&& other) : ptr_(other.ptr_) { std::cout << "move construct..." << std::endl; other.ptr_ = nullptr; }

	inline nontrivial& nontrivial::operator=(const nontrivial& other)
	{
		if (&other != this)
		{
			if (other.ptr_ == nullptr)
			{
				std::cout << "copy assign null" << std::endl;
				ptr_ = nullptr;
			}
			else
			{
				std::cout << "copy assign value: " << (int)other << std::endl;
				ptr_ = new int(*other.ptr_);
			}
		}
		return *this;
	}

	inline nontrivial& nontrivial::operator=(nontrivial&& other)
	{
		std::cout << "move assign..." << std::endl;
		ptr_ = other.ptr_;
		other.ptr_ = nullptr;
		return *this;
	}

	inline nontrivial::~nontrivial()
	{
		if (ptr_ == nullptr) std::cout << "destruct: nullptr" << std::endl;
		else
		{
			std::cout << "destruct: " << *ptr_ << std::endl;
			delete ptr_;
		}
	}

	inline nontrivial::operator int()const
	{
		if (ptr_ == nullptr) throw std::exception("nullptr");
		std::cout << "cast to: " << *ptr_ << std::endl;
		return *ptr_;
	}

	inline std::ostream& operator<<(std::ostream& out, const nontrivial& nt)
	{
		out << "nontrivial ";

		if (nt.ptr_ == nullptr) return out << "nullptr";
		return out << *nt.ptr_;
	}
}


#endif
#endif