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

namespace fcl
{
	//strong typed discriminatory (heterogeneous) container
	template<typename a, typename b, typename ...rest>
	struct variant
	{
		using list = TMP::List<a, b, rest...>;

		friend variant_traits<variant<a, b, rest...>>;

		variant() :index_(-1), ptr_(nullptr) {}

		template<typename T, typename = std::enable_if_t<TMP::elem<list, T>::value>>
		variant(T value) : index_(TMP::elem_index<T, list>::value), ptr_(new T(std::move(value))) {}

		variant(const variant& other) :index_(other.index_) { copy_op(other.ptr_); }

		variant(variant&& other) :index_(other.index_), ptr_(other.ptr_) { other.ptr_ = nullptr; }

		template<typename U, typename std::enable_if_t<TMP::elem<list, U>::value>>
		variant& operator=(U value)
		{
			~variant();
			index_ = TMP::elem_index<U, list>::value;
			ptr_ = new U(std::move(value));
			return *this;
		}

		variant& operator=(const variant& other)
		{
			if (&other != this)
			{
				index_ = other.index_;
				copy_op(other.ptr_);
			}
			return *this;
		}

		variant& operator=(variant&& other)
		{
			index_ = other.index_;
			ptr_ = other.ptr_;
			other.ptr_ = nullptr;
			return *this;
		}

		~variant();


	private:

		template<typename U, typename std::enable_if_t<TMP::elem<list, U>::value>>
		variant(U* ptr) :index_(TMP::elem_index<T, list>::value), ptr_(ptr) {}

		//bisection search convert runtime int to a compile-time one
		//might take a while if variant contains a lot of types
		//for instance for a variant of 5, instantiations would be 5 destruct() and 11 try_destruct()
		//adding copy methods gives us 32 different methods
		template<int N, int M>
		void try_destruct(int i)
		{
			constexpr int pivot = (N + M) / 2;
			if (i == pivot)
				destruct<pivot>();
			else if (i > pivot)
				try_destruct<pivot, M>(i);
			else try_destruct<N, pivot>(i);
		}

		template<int N>
		void destruct()
		{
			typename TMP::index<list, N>::type* actual = (typename TMP::index<list, N>::type*)ptr_;
			delete actual;
			ptr_ = nullptr;
		}

		void copy_op(void* ptr)
		{
			if (index_ == 0)
			{
				copy<0>(ptr);
				return;
			}
			constexpr int last = TMP::length<list>::value - 1;
			if (index_ == last) copy<last>(ptr);
			else try_copy<0, last>(ptr);
		}

		template<int N>
		void copy(void* ptr)
		{
			typename TMP::index<list, N>::type* actual = (typename TMP::index<list, N>::type*)ptr;
			ptr_ = new typename TMP::index<list, N>::type(*actual);
		}

		template<int N, int M>
		void try_copy(void* ptr)
		{
			constexpr int pivot = (N + M) / 2;
			if (index_ == pivot)
				copy<pivot>(ptr);
			else if (index_ > pivot)
				try_copy<pivot, M>(ptr);
			else try_copy<N, pivot>(ptr);
		}

		void * ptr_;
		int index_;
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

	//implenmetaion of dtor
	template<typename a, typename b, typename ...rest>
	inline variant<a, b, rest...>::~variant()
	{
		if (index_ == 0)
		{
			destruct<0>();
			return;
		}
		constexpr int last = TMP::length<list>::value - 1;
		if (index_ == last) destruct<last>();
		else try_destruct<0, last>(index_);
	}
}

#endif