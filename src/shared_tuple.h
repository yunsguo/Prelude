/*
*   shared_tuple.h:
*   A container template for sharing partially applied parameters
*   Language: C++, Visual Studio 2017
*   Platform: Windows 10 Pro
*   Application: recreational
*   Author: Yunsheng Guo, yguo125@syr.edu
*/


/*
*
*   Package Operations:
*	a heterogenerous container that shares the object it contains during copying
*
*   Public Interface:
*   Build Process:
*
*   Maintenance History:
*   August 6
*   first draft
*	August 10
*	removed from project
*	August 30
*	add back to the shared_tuple branch
*
*
*/

#pragma once

#ifndef SHARED_TUPLE_H
#define SHARED_TUPLE_H

#include <type_traits>
#include <memory>
#include <tuple>

namespace details
{

	template<typename a>
	struct is_simple :public std::bool_constant<
		std::is_trivially_constructible<a>::value &&
		std::is_trivially_copy_constructible<a>::value &&
		std::is_trivially_destructible<a>::value &&
		std::is_trivially_copy_assignable<a>::value
	> {};

	template<typename a>
	struct simple_shared
	{
		using is_simple = is_simple<a>;
		using simple = std::conditional_t<is_simple::value, a, std::shared_ptr<a>>;

		simple_shared() :data() {}

		template<typename = std::enable_if_t<is_simple::value>>
		simple_shared(a&& value) : data(std::forward<a>(value)) {}

		template<typename = std::enable_if_t<!is_simple::value>, size_t = 0>
		simple_shared(a&& value) : data() { data = std::make_shared<a>(std::forward<a>(value)); }

		simple_shared(const simple_shared& other) :data(other.data) {}

		simple_shared(simple_shared&& other) :data(std::move(other.data)) {}

		simple_shared& operator=(const simple_shared& other)
		{
			if (&other != this)
				data = other.data;
			return *this;
		}

		simple_shared& operator=(simple_shared&& other)
		{
			data = std::move(other.data);
			return *this;
		}

		template<typename = std::enable_if_t<is_simple::value>>
		const a& value()const { return data; }

		template<typename = std::enable_if_t<!is_simple::value>, size_t = 0>
		const a& value()const { return *data; }

	private:
		simple data;
	};

	template<typename ...as>
	struct shared_tuple;

	template<typename a, typename ...as>
	struct shared_tuple<a, as...>
	{

		template<typename ...bs>
		friend struct shared_tuple;

		shared_tuple():curr_(),next_() {}
		shared_tuple(a&& curr, as&&... next) :curr_(std::forward<a>(curr)), next_(std::forward<as>(next)...) {}
		shared_tuple(simple_shared<a> curr, shared_tuple<as...> next) :curr_(std::move(curr)), next_(std::move(next)) {}

		shared_tuple(const shared_tuple& other) :curr_(other.curr_), next_(other.next_) {}
		shared_tuple(shared_tuple&& other) :curr_(std::move(other.curr_)), next_(std::move(other.next_)) {}

		shared_tuple& operator=(const shared_tuple& other)
		{
			if (&other != this)
			{
				curr_ = other.curr_;
				next_ = other.next_;
			}
			return *this;
		}

		shared_tuple& operator=(shared_tuple&& other)
		{
			curr_ = std::move(other.curr_);
			next_ = std::move(other.next_);
			return *this;
		}

		std::tuple<a, as...> to_tuple()const
		{
			auto first = curr_.value();
			return std::tuple_cat(std::make_tuple<a>(std::move(first)), next_.to_tuple());
		}

		template<typename b>
		shared_tuple<a, as..., b> push_back(b&& value)const
		{
			return shared_tuple<a, as..., b>(curr_,next_.push_back(std::forward<b>(value)));
		}

		template<typename b>
		shared_tuple<b, a, as...> push_front(b&& value)const
		{
			return shared_tuple<b, a, as...>(simple_shared<b>(std::forward<b>(value)), *this);
		}

	private:

		simple_shared<a> curr_;
		shared_tuple<as...> next_;

	};

	template<>
	struct shared_tuple<>
	{
		shared_tuple() = default;

		std::tuple<> to_tuple()const { return std::tuple<>(); }

		template<typename b>
		shared_tuple<b> push_back(b&& value)const { return shared_tuple<b>(std::forward<b>(value)); }

		template<typename b>
		shared_tuple<b> push_front(b&& value)const { return shared_tuple<b>(std::forward<b>(value)); }
	};

	template<typename ...as>
	shared_tuple<as...> make_shared_tuple(as&&... args) { return shared_tuple<as...>(std::forward<as>(args)...); }

};
#endif