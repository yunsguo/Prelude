/*
*   variant.h:
*	previously or.h
*   A container template for implenmeting or syntax, an any type if you will
*   Language: C++, Visual Studio 2017
*   Platform: Windows 10 Pro
*   Application: recreational
*   Author: Yunsheng Guo, yguo125@syr.edu
*/


/*
*
*   Package Operations:
*	can contains a certain type given the potential types
*	can be retrived
*	default constructed instance is ill-formed
*	infer contained type is runtime while infer whether it's legal to contain is compile-time
*	if anyone finds a way to do compile time inference, please contact me
*
*   Public Interface:
*	variant<int,char,float,std::string> v = 5;
*	bool t = v.is_of<int>();
*	int i = v.get<int>();
*	int i1 = v.move<int>(); //v is ill-formed after this
*	auto v2 = v;
*	auto v3(v);
*	//though not encouraged to use its own method outside of this package
*	//use variant trait methods if possible
*
*   Build Process:
*   requires meta.h and TMP.h
*
*   Maintenance History:
*   June 18
*	first draft, based on recursive union with a size_t index
*   August 12
*   refactor due to the fact that the previous union approach is failed when working with shared_tuple.h
*	change to a void pointer approach, easy to implenment, but probabliy a little bit hard on the compiler
*	due to all the instantiations of copy and destruct method
*	August 25
*	final review for publish
*
*
*/

#pragma once
#ifndef _VARIANT_
#define _VARIANT_


#include "meta.h"
#include <exception>
#include <memory>

namespace details
{
	//heterogeneous container
	template<typename a>
	using is_union_member = std::bool_constant<
		std::is_trivially_constructible<a>::value &&
		std::is_trivially_copy_constructible<a>::value &&
		std::is_trivially_destructible<a>::value &&
		std::is_trivially_copy_assignable<a>::value
	>;

	//heterogeneous container
	template<typename list>
	union hetero_container
	{
		using head = typename TMP::head<list>::type;
		using tail = typename TMP::tail<list>::type;
		using curr_type = std::conditional_t<is_union_member<head>::value, head, head*>;

		template<typename a>
		using elem = TMP::elem<list, a>;

		hetero_container() {}

		template<typename a, typename = std::enable_if_t<std::is_same<a, head>::value && is_union_member<a>::value>>
		hetero_container(a&& value) : curr_(std::forward<a>(value)) {}

		template<typename a, typename = std::enable_if_t<std::is_same<a, head>::value && !is_union_member<a>::value>, size_t = 0>
		hetero_container(a&& value) : curr_(new a(std::forward<a>(value))) {}

		template<typename a, typename = std::enable_if_t<!std::is_same<a, head>::value>, size_t = 0, size_t = 1>
		hetero_container(a&& value) : next_(std::forward<a>(value)) {}

		template<typename a, typename = std::enable_if_t<std::is_same<a, head>::value && is_union_member<a>::value>>
		a& ref() { return curr_; }

		template<typename a, typename = std::enable_if_t<std::is_same<a, head>::value && !is_union_member<a>::value>, size_t = 0>
		a& ref() { return *curr_; }

		template<typename a, typename = std::enable_if_t<!std::is_same<a, head>::value>, size_t = 0, size_t = 1>
		a& ref() { return next_.ref<a>(); }

		template<typename a, typename = std::enable_if_t<std::is_same<a, head>::value && is_union_member<a>::value>>
		void destruct() { curr_.~a(); }

		template<typename a, typename = std::enable_if_t<std::is_same<a, head>::value && !is_union_member<a>::value>, size_t = 0>
		void destruct() { delete curr_; curr_ = nullptr; }

		template<typename a, typename = std::enable_if_t<!std::is_same<a, head>::value>, size_t = 0, size_t = 1>
		void destruct() { next_.destruct<a>(); }

		~hetero_container() {}

	private:
		curr_type curr_;
		hetero_container<tail> next_;
	};

	template<typename T>
	union hetero_container<TMP::Cons<T, TMP::Nil>>
	{
		using curr_type = std::conditional_t<is_union_member<T>::value, T, T*>;
		using head = T;

		hetero_container() {}

		template<typename = std::enable_if_t<is_union_member<head>::value>>
		hetero_container(head&& value) : curr_(std::forward<head>(value)) {}

		template<typename = std::enable_if_t<!is_union_member<head>::value>, size_t = 0>
		hetero_container(head&& value) : curr_(new head(std::forward<head>(value))) {}

		template<typename a, typename = std::enable_if_t<std::is_same<a, head>::value && is_union_member<a>::value>>
		a& ref() { return curr_; }

		template<typename a, typename = std::enable_if_t<std::is_same<a, head>::value && !is_union_member<a>::value>, size_t = 0>
		a& ref() { return *curr_; }

		template<typename a, typename = std::enable_if_t<std::is_same<a, head>::value && is_union_member<a>::value>>
		void destruct() { curr_.~a(); }

		template<typename a, typename = std::enable_if_t<std::is_same<a, head>::value && !is_union_member<a>::value>, size_t = 0>
		void destruct() { delete curr_; curr_ = nullptr; }

		~hetero_container() {}

	private:
		curr_type curr_;
	};

}

namespace fcl
{
	//strong typed discriminatory (heterogeneous) container
	template<typename a, typename b, typename ...rest>
	struct variant
	{
		using list = TMP::List<a, b, rest...>;

		variant() :index_(-1) {}

		template<typename T, typename = std::enable_if_t<TMP::elem<list, T>::value>>
		variant(T value) : index_(TMP::elem_index<T, list>::value), hc_(std::move(value)) {}

		variant(const variant& other) :index_(other.index_), hc_() {}

		variant(variant&& other) :index_(other.index_), hc_() {}

		template<typename U, typename std::enable_if_t<TMP::elem<list, U>::value>>
		variant& operator=(U rvalue)
		{
			index_ = TMP::elem_index<U, list>::value;
			hc_.movein(std::move(other.hc_), index_);
			return *this;
		}

		variant& operator=(const variant& other)
		{
			if (&other != this)
			{
				index_ = other.index_;
				hc_.copyin(other.hc_, index_);
			}
			return *this;
		}

		variant& operator=(variant&& other)
		{
			index_ = other.index_;
			hc_.movein(std::move(other.hc_), index_);
			return *this;
		}

		~variant() {}

	private:
		details::hetero_container<list> hc_;
		size_t index_;

		template<size_t N, size_t M>
		void try_destruct()
		{
			constexpr size_t pivot = (N + M) / 2;
			if (index_ == pivot)
				destruct<pivot>();
			else if (i > pivot)
				try_destruct<pivot, M>(i);
			else try_destruct<N, pivot>(i);
		}

		template<size_t I>
		void destruct() { hc_.destruct<typename TMP::index<list, I>::type>(); }

		template<size_t I>
		void movein(details::hetero_container<list>& hc)
		{
			using T = typename TMP::index<list, I>::type;
			hc_.ref<T>() = 
		}
	};

	//variant variant traits implenmentation
	template<typename a, typename b, typename ...rest>
	struct variant_traits<variant<a, b, rest...>>
	{

		using possess = std::true_type;

		using has_default = std::false_type;

		using def = variant<a, b, rest...>;

		using list = TMP::List<a, b, rest...>;

		template<typename T>
		using elem = TMP::elem<list, T>;

		template<typename T, typename = std::enable_if_t<elem<T>::value>>
		static bool is_of(const def& var) { return var.index_ == TMP::elem_index<T, list>::value; }

		template<typename T, typename = std::enable_if_t<!elem<T>::value>, size_t = 0>
		constexpr static bool is_of(const def& var) { return false; }

		template<typename T>
		static T&& move(def& var)
		{
			if (!is_of<T>(var))  throw std::exception("error: type mismatch.");
			T* actual = (T*)var.ptr_;
			return std::move(*actual);
		}

		template<typename T>
		static const T& get(const def& var)
		{
			if (!is_of<T>(var))  throw std::exception("error: type mismatch.");
			T* actual = (T*)var.ptr_;
			return *actual;
		}
	};
}

#endif