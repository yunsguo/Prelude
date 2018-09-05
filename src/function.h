/*
*   function.h:
*   A container template for generic functional operation
*   Language: C++, Visual Studio 2017
*   Platform: Windows 10 Pro
*   Application: recreational
*   Author: Yunsheng Guo, yguo125@syr.edu
*/


/*
*
*   Package Operations:
*	takes a function pointer in the form of r function(a first, as...rest)
*	or a type which can be converted as such
*	does not take lambda with capture
*	call with operator()
*	partial or fullly apply with operator<<
*	monadic partial apply (reversed order) with operator>>=
*
*   Public Interface:
*	Function<int,int> k = unary_function;
*	Function<int,int> l(unary_function);
*	Function<int,int,int,int,int> m(sum_4);
*	l = k;
*	k = std::move(l);
*	int r = k(1);
*	int r1 = k << 1;
*	int r2 = m << 1 << 2 << 3 << 4;
*	Function<int,int,int> am = m << 1 << 2;
*	int r3 = am(3,4);
*	int r4 = 4 >>= 3 >>= 2 >>= 1 >>= m;
*   int r5 = 4 >>= 3 >>= am;
*	int r6 = am.apply_r(3).apply_r(4); //is not encouraged
*
*   Build Process:
*   requires meta.h
*
*   Maintenance History:
*   early June
*   First draft, build with multiple types and glued together with function_trait
*   August 16
*   refactor by type erasure
*	August 25
*	final review for publish
*
*
*/

#pragma once
#ifndef _FUNCTION_
#define _FUNCTION_

#include "meta.h"

#if _HAS_CXX17
#else

namespace details
{
	template<class F, class Tuple, std::size_t... I>
	constexpr decltype(auto) apply_impl(F&& f, Tuple&& t, std::index_sequence<I...>)
	{
		return std::invoke(std::forward<F>(f), std::get<I>(std::forward<Tuple>(t))...);
	}

}

namespace std
{
	template <class F, class Tuple>
	constexpr decltype(auto) apply(F&& f, Tuple&& t)
	{
		return details::apply_impl(
			std::forward<F>(f), std::forward<Tuple>(t),
			std::make_index_sequence<std::tuple_size_v<std::remove_reference_t<Tuple>>>{});
	}
}

#endif

namespace details
{

	template<typename pack_of_a>
	struct decays;

	template<typename a, typename ...as>
	struct decays<TMP::Pack<a, as...>>
	{
		using type = TMP::cons_t<TMP::Pack<std::decay_t<a>>, typename decays<TMP::Pack<as...>>::type>;
	};

	template<>
	struct decays<TMP::Pack<>> { using type = TMP::Pack<>; };

	template<typename r, typename pack_of_a>
	struct inferred_helper;

	template<typename r, typename ...as>
	struct inferred_helper<r, TMP::Pack<as...>> { static r invoke(as...arg); };

	template<typename r, typename ...as>
	struct Inferred { using type = inferred_helper<r, typename decays<TMP::Pack<as...>>::type>; };

	//template<typename r, typename ...as>
	//struct Inferred { static r invoke(as...arg); };

	template<typename r, typename ...as>
	using func_ptr = decltype(&Inferred<r, as...>::type::invoke);

	template<size_t front, size_t back, typename ...as>
	using inferred_para_list = typename TMP::take<typename TMP::drop<TMP::List<as...>, front>::type, TMP::length<TMP::List<as...>>::value - front - back>::type;

	//functor operator overload and invoke interface
	template<typename r, typename list_of_a>
	struct applicable
	{
		using is_unary = std::bool_constant<TMP::length<list_of_a>::value == 1>;

		using front = TMP::head_t<list_of_a>;

		using back = TMP::last_t<list_of_a>;

		using front_applied_type = typename std::conditional<is_unary::value, r, applicable<r, typename TMP::tail<list_of_a>::type>*>::type;

		using back_applied_type = typename std::conditional<is_unary::value, r, applicable<r, typename TMP::init<list_of_a>::type>*>::type;

		using para_tuple = typename TMP::to_tuple<list_of_a>::type;

		virtual r invoke(para_tuple&& tuple)const = 0;

		virtual front_applied_type push_front(front&& arg)const = 0;

		virtual back_applied_type push_back(back&& arg)const = 0;

		virtual applicable* new_ptr()const = 0;

		virtual ~applicable() {}
	};
}



namespace details
{
	//function concept concrete instance for type erasure
	template<size_t fi, size_t bi, typename r, typename ...as>
	struct func_container;

	template<size_t fi, size_t bi, typename r, typename ...as>
	struct func_container :public details::applicable<r, details::inferred_para_list<fi, bi, as...>>
	{
		using applicable = details::applicable<r, details::inferred_para_list<fi, bi, as...>>;

		using para_list = TMP::List<as...>;

		enum { length = sizeof...(as) };

		using front_tuple = typename TMP::to_tuple<typename TMP::take<para_list, fi>::type>::type;

		using back_tuple = typename TMP::to_tuple<typename TMP::drop<para_list, length - bi>::type>::type;

		using para_tuple = typename applicable::para_tuple;

		using front = typename applicable::front;

		using back = typename applicable::back;

		using front_applied_type = typename std::conditional<applicable::is_unary::value, r, func_container<fi + 1, bi, r, as...>*>::type;

		using back_applied_type = typename std::conditional<applicable::is_unary::value, r, func_container<fi, bi + 1, r, as...>*>::type;

		func_container() = delete;

		template<typename T, typename = std::enable_if_t<std::is_convertible<T, details::func_ptr<r, as...>>::value>>
		func_container(T p, front_tuple&& ft, back_tuple&& bt) :ptr_(p), ft_(std::forward<front_tuple>(ft)), bt_(std::forward<back_tuple>(bt)) {}

		func_container(const func_container& other) :ptr_(other.ptr_), ft_(other.ft_), bt_(other.bt_) {}

		template<typename = std::enable_if_t<applicable::is_unary::value>>
		front_applied_type front_apply(front&& arg) const { return invoke(std::make_tuple(std::forward<front>(arg))); }

		template<typename = std::enable_if_t<!applicable::is_unary::value>, size_t = 0>
		front_applied_type front_apply(front&& arg) const
		{
			auto ft(ft_);
			auto bt(bt_);
			return new func_container<fi + 1, bi, r, as...>(ptr_, std::tuple_cat(ft, std::make_tuple(arg)), std::move(bt));
		}

		template<typename = std::enable_if_t<applicable::is_unary::value>>
		back_applied_type back_apply(back&& arg) const { return invoke(std::make_tuple(arg)); }

		template<typename = std::enable_if_t<!applicable::is_unary::value>, size_t = 0>
		back_applied_type back_apply(back&& arg) const
		{
			auto ft(ft_);
			auto bt(bt_);
			return new func_container<fi, bi + 1, r, as...>(ptr_, std::move(ft), std::tuple_cat(std::make_tuple(arg), bt));
		}

		r invoke(para_tuple&& t)const override
		{
			return std::apply(ptr_, std::tuple_cat(ft_, std::forward<para_tuple>(t), bt_));
		}

		typename applicable::front_applied_type push_front(front&& arg)const override
		{
			return front_apply(std::forward<front>(arg));
		}

		typename applicable::back_applied_type push_back(back&& arg)const override
		{
			return back_apply(std::forward<back>(arg));
		}

		applicable* new_ptr()const override { return new func_container(*this); }

		virtual ~func_container() override {}

	private:
		details::func_ptr<r, as...> ptr_;
		front_tuple ft_;
		back_tuple bt_;
	};

	template<typename r, typename ...as>
	struct func_container<0, 0, r, as...> :public details::applicable<r, TMP::List<as...>>
	{
		using applicable = details::applicable<r, TMP::List<as...>>;

		using para_list = TMP::List<as...>;

		enum { length = sizeof...(as) };

		using para_tuple = typename applicable::para_tuple;

		using front = typename applicable::front;

		using back = typename applicable::back;

		using front_applied_type = typename std::conditional<applicable::is_unary::value, r, func_container<1, 0, r, as...>*>::type;

		using back_applied_type = typename std::conditional<applicable::is_unary::value, r, func_container<0, 1, r, as...>*>::type;

		func_container() = delete;

		template<typename T, typename = std::enable_if_t<std::is_convertible<T, details::func_ptr<r, as...>>::value>>
		func_container(T p) :ptr_(p) {}

		func_container(const func_container& other) :ptr_(other.ptr_) {}

		template<typename = std::enable_if_t<applicable::is_unary::value>>
		front_applied_type front_apply(front&& arg) const { return invoke(std::make_tuple(std::forward<front>(arg))); }

		template<typename = std::enable_if_t<!applicable::is_unary::value>, size_t = 0>
		front_applied_type front_apply(front&& arg) const
		{
			return new func_container<1, 0, r, as...>(ptr_, std::make_tuple(std::forward<front>(arg)));
		}

		template<typename = std::enable_if_t<applicable::is_unary::value>>
		back_applied_type back_apply(back&& arg) const { return invoke(std::make_tuple(std::forward<back>(arg))); }

		template<typename = std::enable_if_t<!applicable::is_unary::value>, size_t = 0>
		back_applied_type back_apply(back&& arg) const
		{
			return new func_container<0, 1, r, as...>(ptr_, std::make_tuple(std::forward<back>(arg)));
		}

		r invoke(para_tuple&& t)const override
		{
			return std::apply(ptr_, std::forward<para_tuple>(t));
		}

		typename applicable::front_applied_type push_front(front&& arg)const override
		{
			return front_apply(std::forward<front>(arg));
		}

		typename applicable::back_applied_type push_back(back&& arg)const override
		{
			return back_apply(std::forward<back>(arg));
		}

		applicable* new_ptr()const override { return new func_container(*this); }

		virtual ~func_container() override {}

	private:
		details::func_ptr<r, as...> ptr_;
	};

	template<size_t fi, typename r, typename ...as>
	struct func_container<fi,0,r,as...> :public details::applicable<r, details::inferred_para_list<fi, 0, as...>>
	{
		using applicable = details::applicable<r, details::inferred_para_list<fi, 0, as...>>;

		using para_list = TMP::List<as...>;

		enum { length = sizeof...(as) };

		using front_tuple = typename TMP::to_tuple<typename TMP::take<para_list, fi>::type>::type;

		using para_tuple = typename applicable::para_tuple;

		using front = typename applicable::front;

		using back = typename applicable::back;

		using front_applied_type = typename std::conditional<applicable::is_unary::value, r, func_container<fi + 1, 0, r, as...>*>::type;

		using back_applied_type = typename std::conditional<applicable::is_unary::value, r, func_container<fi, 1, r, as...>*>::type;

		func_container() = delete;

		template<typename T, typename = std::enable_if_t<std::is_convertible<T, details::func_ptr<r, as...>>::value>>
		func_container(T p, front_tuple&& ft) :ptr_(p), ft_(std::forward<front_tuple>(ft)) {}

		func_container(const func_container& other) :ptr_(other.ptr_), ft_(other.ft_) {}

		template<typename = std::enable_if_t<applicable::is_unary::value>>
		front_applied_type front_apply(front&& arg) const { return invoke(std::make_tuple(std::forward<front>(arg))); }

		template<typename = std::enable_if_t<!applicable::is_unary::value>, size_t = 0>
		front_applied_type front_apply(front&& arg) const
		{
			auto ft(ft_);
			return new func_container<fi + 1, 0, r, as...>(ptr_, std::tuple_cat(ft, std::make_tuple(arg)));
		}

		template<typename = std::enable_if_t<applicable::is_unary::value>>
		back_applied_type back_apply(back&& arg) const { return invoke(std::make_tuple(arg)); }

		template<typename = std::enable_if_t<!applicable::is_unary::value>, size_t = 0>
		back_applied_type back_apply(back&& arg) const
		{
			auto ft(ft_);
			return new func_container<fi, 1, r, as...>(ptr_, std::move(ft), std::make_tuple(std::forward<back>(arg)));
		}

		r invoke(para_tuple&& t)const override
		{
			return std::apply(ptr_, std::tuple_cat(ft_, std::forward<para_tuple>(t)));
		}

		typename applicable::front_applied_type push_front(front&& arg)const override
		{
			return front_apply(std::forward<front>(arg));
		}

		typename applicable::back_applied_type push_back(back&& arg)const override
		{
			return back_apply(std::forward<back>(arg));
		}

		applicable* new_ptr()const override { return new func_container(*this); }

		virtual ~func_container() override {}

	private:
		details::func_ptr<r, as...> ptr_;
		front_tuple ft_;
	};

	template<size_t bi, typename r, typename ...as>
	struct func_container<0,bi,r,as...> :public details::applicable<r, details::inferred_para_list<0, bi, as...>>
	{
		using applicable = details::applicable<r, details::inferred_para_list<0, bi, as...>>;

		using para_list = TMP::List<as...>;

		enum { length = sizeof...(as) };

		using back_tuple = typename TMP::to_tuple<typename TMP::drop<para_list, length - bi>::type>::type;

		using para_tuple = typename applicable::para_tuple;

		using front = typename applicable::front;

		using back = typename applicable::back;

		using front_applied_type = typename std::conditional<applicable::is_unary::value, r, func_container<1, bi, r, as...>*>::type;

		using back_applied_type = typename std::conditional<applicable::is_unary::value, r, func_container<0, bi + 1, r, as...>*>::type;

		func_container() = delete;

		template<typename T, typename = std::enable_if_t<std::is_convertible<T, details::func_ptr<r, as...>>::value>>
		func_container(T p, back_tuple&& bt) :ptr_(p), bt_(std::forward<back_tuple>(bt)) {}

		func_container(const func_container& other) :ptr_(other.ptr_), bt_(other.bt_) {}

		template<typename = std::enable_if_t<applicable::is_unary::value>>
		front_applied_type front_apply(front&& arg) const { return invoke(std::make_tuple(std::forward<front>(arg))); }

		template<typename = std::enable_if_t<!applicable::is_unary::value>, size_t = 0>
		front_applied_type front_apply(front&& arg) const
		{
			auto bt(bt_);
			return new func_container<1, bi, r, as...>(ptr_, std::make_tuple(std::forward<front>(arg)), std::move(bt));
		}

		template<typename = std::enable_if_t<applicable::is_unary::value>>
		back_applied_type back_apply(back&& arg) const { return invoke(std::make_tuple(std::forward<back>(arg))); }

		template<typename = std::enable_if_t<!applicable::is_unary::value>, size_t = 0>
		back_applied_type back_apply(back&& arg) const
		{
			auto bt(bt_);
			return new func_container<0, bi + 1, r, as...>(ptr_, std::tuple_cat(std::make_tuple(arg), bt));
		}

		r invoke(para_tuple&& t)const override
		{
			return std::apply(ptr_, std::tuple_cat(std::forward<para_tuple>(t), bt_));
		}

		typename applicable::front_applied_type push_front(front&& arg)const override
		{
			return front_apply(std::forward<front>(arg));
		}

		typename applicable::back_applied_type push_back(back&& arg)const override
		{
			return back_apply(std::forward<back>(arg));
		}

		applicable* new_ptr()const override { return new func_container(*this); }

		virtual ~func_container() override {}

	private:
		details::func_ptr<r, as...> ptr_;
		back_tuple bt_;
	};

	template<typename r, typename pack_of_a>
	struct conversion_helper;

	template<typename r, typename ...as>
	struct conversion_helper<r, TMP::Pack<as...>> { using type = fcl::Function<r, as...>; };

	template<typename f>
	struct reverse_apply;

	template<typename r, typename a, typename b, typename ...rest>
	struct reverse_apply<fcl::Function<r, a, b, rest...>> { using type = typename conversion_helper<r, TMP::to_pack_t<TMP::init_t<TMP::List<a, b, rest...>>>>::type; };

	template<typename r, typename a>
	struct reverse_apply<fcl::Function<r, a>> { using type = r; };
}

namespace fcl
{

	//function container definition
	template<typename r, typename a, typename b, typename ...rest>
	struct Function<r, a, b, rest...>
	{
		template<typename r1, typename b1, typename ...bs>
		friend struct Function;
		template<typename f1>
		friend struct function_traits;

	private:
		using func_ptr = details::func_ptr<r, a, b, rest...>;
		using func_con = details::func_container<0, 0, r, a, b, rest...>;
		using applicable = details::applicable<r, TMP::List<a, b, rest...>>;

	public:

		using monadic_applied = monadic_applied_type<Function>;
		using last = last_parameter<Function>;

		template<typename f, typename = std::enable_if_t<std::is_convertible<f, func_ptr>::value>>
		Function(f ptr) :ptr_(new func_con(ptr)) {}

		Function(const Function& other) :ptr_(other.ptr_->new_ptr()) {}

		Function(Function&& other) :ptr_(other.ptr_) { other.ptr_ = nullptr; }

		Function& operator=(const Function& other)
		{
			if (&other != this)
				ptr_ = other.ptr_->new_ptr();
			return *this;
		}

		Function& operator=(Function && other)
		{
			ptr_ = other.ptr_;
			other.ptr_ = nullptr;
			return *this;
		}

		r operator()(a first, b second, rest...args)const { return ptr_->invoke(std::forward_as_tuple(first, second, args...)); }

		~Function() { delete ptr_; }


	private:

		applicable * ptr_;

		Function() :ptr_(nullptr) {};
		Function(applicable* ptr) :ptr_(ptr) {};
	};

	//unary function container definition
	template<typename r, typename a>
	struct Function<r, a>
	{

		template<typename r1, typename b1, typename ...bs>
		friend struct Function;
		template<typename f1>
		friend struct function_traits;

	private:
		using func_ptr = details::func_ptr<r, a>;
		using func_con = details::func_container<0, 0, r, a>;
		using applicable = details::applicable<r, TMP::List<a>>;

	public:

		template<typename f, typename = std::enable_if_t<std::is_convertible<f, func_ptr>::value>>
		Function(f ptr) :ptr_(new func_con(ptr)) {}

		Function(const Function& other) :ptr_(other.ptr_->new_ptr()) {}

		Function(Function&& other) :ptr_(other.ptr_) { other.ptr_ = nullptr; }

		Function& operator=(const Function& other)
		{
			if (&other != this)
				ptr_ = other.ptr_->new_ptr();
			return *this;
		}

		Function& operator=(Function&& other)
		{
			ptr_ = other.ptr_;
			other.ptr_ = nullptr;
			return *this;
		}

		r operator()(a arg)const { return ptr_->invoke(std::forward_as_tuple(arg)); }

		~Function() { delete ptr_; }

	private:

		applicable * ptr_;

		Function() :ptr_(nullptr) {};
		Function(applicable* ptr) :ptr_(ptr) {};
	};

	//function type trait definition
	template<typename r, typename a, typename b, typename ...rest>
	struct function_traits<Function<r, a, b, rest...>>
	{
		using f = Function<r, a, b, rest...>;
		using possess = std::true_type;
		using type = TMP::Pack<a, b, rest..., r>;
		using applied = Function<r, b, rest...>;
		using monadic_applied = typename details::reverse_apply<Function<r, a, b, rest...>>::type;
		using head = a;
		using last = TMP::last_t<TMP::Pack<a, b, rest...>>;

		static applied apply(const f& func, const head& arg)
		{
			head arg_(arg);
			return applied(func.ptr_->push_front(std::move(arg_)));
		}

		static monadic_applied monadic_apply(const f& func, const last& arg)
		{
			last arg_(arg);
			return monadic_applied(func.ptr_->push_back(std::move(arg_)));
		}
	};

	//unary function type trait definition
	template<typename r, typename a>
	struct function_traits<Function<r, a>>
	{
		using f = Function<r, a>;
		using possess = std::true_type;
		using type = TMP::Pack<a, r>;
		using applied = r;
		using monadic_applied = r;
		using head = a;
		using last = a;

		static r apply(const f& func, const a& arg)
		{
			a arg_(arg);
			return func.ptr_->invoke(std::make_tuple<a>(std::move(arg_)));
		}

		static r monadic_apply(const f& func, const a& arg)
		{
			a arg_(arg);
			return func.ptr_->invoke(std::make_tuple<a>(std::move(arg_)));
		}
	};
}


#endif // !FUNCTION_H
