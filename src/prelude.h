/*
*	prelude.h:
*   emulating the Haskell prelude
*   Language: C++, Visual Studio 2017
*   Platform: Windows 10 Pro
*   Application: recreational
*   Author: Yunsheng Guo, yguo125@syr.edu
*/


/*
*
*   Package Operations:
*	defines some convenient aliases
*	includes type class implenmetations, Maybe type and its type traits implenmentations
*	some static operators for type classes
*	pattern matching snytax
*
*   Public Interface:
*	check Haskell prelude for futher info
*	Eq, Ord, Show all have their operator implenmeted
*	Functor fmap is implenmented
*	Applicative has the same partial apply operator as Function type
*	Monad has a unique operator >>= for sequence and operator >> for compose
*	due to the order needed to achieve do Haskell notation
*	Monad is no longer an Applicative
*
*   Build Process:
*   requires function.h and variant.h
*
*   Maintenance History:
*   June 6
*	first draft contains typeclass definitions and function application definition
*   August 12
*   refactor. move Maybe type into this for clarity and move all type class and traits declaration to meta.h
*	change pattern matching syntax and let pattern matching use Maybe
*	August 25
*	final review for publish
*
*
*/

#pragma once
#ifndef _PRELUDE_
#define _PRELUDE_

#include <string>
#include "function.h"
#include "variant.h"

namespace details
{
	template<typename ...as>
	struct first_true_index;

	template<typename a>
	struct first_true_index<a> { enum { value = a::value ? 0 : 1 }; };

	template<typename a, typename ...as>
	struct first_true_index<a, as...> { enum { value = a::value ? 0 : 1 + first_true_index<as...>::value }; };

	template<std::size_t... Ns, typename... Ts>
	auto tail_impl(std::index_sequence<Ns...>, std::tuple<Ts...>&& t)
	{
		return  std::make_tuple(std::get<Ns + 1u>(t)...);
	}

	template <typename... Ts >
	auto tail(std::tuple<Ts...>&& t)
	{
		return  tail_impl(std::make_index_sequence<sizeof...(Ts) - 1u>(), std::forward<std::tuple<Ts...>>(t));
	}

	template<typename list1, typename list2>
	struct are_legal;

	template<typename a, typename b, typename c, typename d>
	struct are_legal<TMP::Cons<a, b>, TMP::Cons<c, d>> :public std::bool_constant<fcl::variant_traits<a>::elem<c>::value && are_legal<b, d>::value> {};

	template<>
	struct are_legal<TMP::Nil, TMP::Nil> :public std::true_type {};

	template<typename a, typename b>
	struct are_legal<TMP::Cons<a, b>, TMP::Nil> :public std::false_type {};

	template<typename a, typename b>
	struct are_legal<TMP::Nil, TMP::Cons<a, b>> :public std::false_type {};

	template<typename a, typename ...as>
	struct are_show;

	template<typename a>
	struct are_show<a> :public std::bool_constant<fcl::Show<a>::pertain::value> {};

	template<typename a, typename ...as>
	struct are_show :public std::bool_constant<fcl::Show<a>::pertain::value && are_show<as...>::value> {};

	template<typename a, typename...as>
	struct pattern
	{
		template<typename b, typename ...bs>
		static bool match(a first, as...args) { return variant_traits<a>::is_of<b>(first) && pattern<as...>::match<bs...>(args...); }

		template<typename b, typename ...bs>
		static std::tuple<b, bs...> gets(a first, as...args) { return std::tuple_cat(std::make_tuple<b>(variant_traits<a>::move<b>(first)), pattern<as...>::gets<bs...>(args...)); }
	};

	template<typename a>
	struct pattern<a>
	{
		template<typename b>
		static bool match(a first) { return variant_traits<a>::is_of<b>(first); }

		template<typename b>
		static std::tuple<b> gets(a first) { return std::make_tuple<b>(variant_traits<a>::move<b>(first)); }
	};
}

namespace fcl
{

	template<typename a, typename b, typename ...rest>
	using Data = variant<a, b, rest...>;

	//generic list haskell conter-part(double linked list)
	template<typename a>
	using List = std::list<a>;

	//generic list cons method
	template<typename a>
	List<a> cons(a first, List<a> as)
	{
		as.push_front(first);
		return as;
	}


	//generic list uncons method
	template<typename a>
	Pair<a, List<a>> uncons(List<a> as)
	{
		a f = as.front();
		as.pop_front();
		return std::make_pair<a, List<a>>(std::move(f), std::move(as));
	}

	template<typename a>
	a id(a value) { return std::move(value); }

	template<typename a>
	struct Eq
	{
		using pertain = std::bool_constant<std::is_arithmetic<a>::value || Ord<a>::pertain::value>;

		template<typename = std::enable_if_t<std::is_arithmetic<a>::value>>
		static bool equals(a one, a other) { return one == other; }

		template<typename = std::enable_if_t<!std::is_arithmetic<a>::value && Ord<a>::pertain::value>, size_t = 0>
		static bool equals(a one, a other) { return Ord<a>::compare(one, other) == Ordering::EQ; }
	};

	template<typename a, typename = std::enable_if_t<Eq<a>::pertain::value && !std::is_arithmetic<a>::value>>
	bool operator==(const a& one, const a& other) { return Eq<a>::equals(one, other); }

	template<typename a, typename = std::enable_if_t<Eq<a>::pertain::value && !std::is_arithmetic<a>::value>>
	bool operator!=(const a& one, const a& other) { return !Eq<a>::equals(one, other); }

	template<typename a>
	struct Ord
	{
		using pertain = std::is_arithmetic<a>;
		static Ordering compare(a one, a other)
		{
			if (one > other)return Ordering::GT;
			if (one == other) return Ordering::EQ;
			return Ordering::LT;
		}
	};


	template<typename a, typename = std::enable_if_t<Ord<a>::pertain::value>>
	bool operator<=(const a& one, const a& other) { return Ord<a>::compare(one, other) != Ordering::GT; }

	template<typename a, typename = std::enable_if_t<Ord<a>::pertain::value>>
	bool operator>=(const a& one, const a& other) { return Ord<a>::compare(one, other) != Ordering::LT; }

	template<typename a, typename = std::enable_if_t<Ord<a>::pertain::value>>
	bool operator<(const a& one, const a& other) { return Ord<a>::compare(one, other) == Ordering::LT; }

	template<typename a, typename = std::enable_if_t<Ord<a>::pertain::value>>
	bool operator>(const a& one, const a& other) { return Ord<a>::compare(one, other) == Ordering::GT; }

	template<typename a>
	struct Show
	{
		using pertain = std::is_arithmetic<a>;

		template<typename = std::enable_if_t<pertain::value>>
		static std::string show(const a& value) { return std::to_string(value); }
	};

#ifdef _IOSTREAM_
	template<typename a, typename = std::enable_if_t<Show<a>::pertain::value && !std::is_same<a, std::string>::value && !std::is_arithmetic<a>::value>>
	std::ostream& operator<<(std::ostream& out, const a& value) { return out << Show<a>::show(value); }
#endif

	template<template<typename> typename F>
	struct Functor
	{
		using pertain = typename Applicative<F>::pertain;

		template<typename f, typename = std::enable_if_t<is_function<f>::value && Applicative<F>::pertain::value>>
		static F<applied_type<f>> fmap(const f& func, const F<head_parameter<f>>& fa)
		{
			return Applicative<F>::template sequence<f>(Applicative<F>::template pure<f>(func), fa);
		}
	};

	template<template<typename> typename A>
	struct Applicative
	{
		using pertain = std::false_type;

		template<typename a>
		static A<a> pure(const a&);

		template<typename f, typename = std::enable_if_t<is_function<f>::value>>
		static A<applied_type<f>> sequence(const A<f>&, const A<head_parameter<f>>&);
	};

	template<template<typename> typename A, typename f, typename = std::enable_if_t<Applicative<A>::pertain::value && is_function<f>::value>>
	A<applied_type<f>> operator<<(const A<f>& af, const A<head_parameter<f>>& aa) { return Applicative<A>::sequence<f>(af, aa); }

	template<template<typename> typename A>
	struct Alternative
	{
		using pertain = std::false_type;

		template<typename a>
		static A<a> empty();

		template<typename a>
		static A<a> alter(const A<a>&, const A<a>&);
	};

	template<template<typename> typename A, typename a, typename = std::enable_if_t<Alternative<A>::pertain::value>>
	A<a> operator||(const A<a>& p, const A<a>& q) { return Alternative<A>::alter(p, q); }

	template<template<typename> typename M>
	struct Monad
	{
		using pertain = std::false_type;

		template<typename a>
		static M<a> pure(const a&);

		template<typename f, typename = std::enable_if_t<is_function<f>::value>>
		static M<monadic_applied_type<f>> sequence(const M<last_parameter<f>>&, const M<f>&);

		template<typename a, typename b>
		static M<b> compose(const M<a>&, const M<b>&);
	};

	template<template<typename> typename M, typename f, typename = std::enable_if_t<is_function<f>::value && Monad<M>::pertain::value>>
	M<monadic_applied_type<f>> operator>>=(const M<last_parameter<f>>& ma, const M<f>& func) { return Monad<M>::sequence<f>(ma, func); }

	template<template<typename> typename M, typename a, typename b, typename = std::enable_if_t<Monad<M>::pertain::value>>
	M<b> operator>>(const M<a>& p, const M<b>& q) { return Monad<M>::compose(p, q); }

	template<typename a>
	struct Just
	{
		a value;
	};

	struct Nothing {};

	//maybe implenmentation
	template<typename a>
	struct Maybe
	{
		Maybe() :value(Nothing()) {}

		Maybe(Just<a> just) :value(just) {}
		Maybe(Nothing n) :value(n) {}

		Maybe(a v_) :value(Just<a>{v_}) {}

		friend variant_traits<Maybe<a>>;

	private:
		Data<Nothing, Just<a>> value;
	};

	template<typename a, typename b>
	b maybe(b default_r, const Function<b, a>& f, const Maybe<a>& ma)
	{
		if (isNothing(ma)) return default_r;
		return f(fromJust(ma));
	}

	template<typename a>
	bool isJust(const Maybe<a>& ma) { return variant_traits<Maybe<a>>::is_of<Just<a>>(ma); }

	template<typename a>
	bool isNothing(Maybe<a> ma) { return variant_traits<Maybe<a>>::is_of<Nothing>(ma); }

	template<typename a>
	a fromJust(Maybe<a> ma) { return variant_traits<Maybe<a>>::move<Just<a>>(ma).value; }

	template<typename a>
	a fromMaybe(a default_r, Maybe<a> ma) { isNothing(ma) ? default_r : variant_traits<Maybe<a>>::move<Just<a>>(ma).value; }

	//Maybe variant traits implenmentation
	template<typename a>
	struct variant_traits<Maybe<a>>
	{
		using possess = std::true_type;
		using has_default = std::true_type;
		using default_type = a;

		using def = Maybe<a>;

		template<typename b>
		using elem = TMP::elem<TMP::List<Nothing, Just<a>>, b>;

		using D = Data<Nothing, Just<a>>;

		template<typename b>
		static bool is_of(const def& maybe) { return variant_traits<D>::is_of<b>(maybe.value); }

		template<typename b>
		static b&& move(def& maybe) { return variant_traits<D>::move<b>(maybe.value); }

		template<typename b>
		static const b& get(const def& maybe) { return variant_traits<D>::get<b>(maybe.value); }
	};

	//Maybe Eq typeclass implenmentation
	template<typename a>
	struct Eq<Maybe<a>>
	{
		using pertain = typename Eq<a>::pertain;

		template<typename = std::enable_if_t<pertain::value>>
		static bool equals(Maybe<a> one, Maybe<a> other)
		{
			if (isJust(one) && isJust(other)) return Eq<a>::equals(fromJust(one), fromJust(other));
			if (isNothing(one) && isNothing(other)) return true;
			return false;
		}
	};

	//Maybe Ord typeclass implenmentation
	template<typename a>
	struct Ord<Maybe<a>>
	{
		using pertain = typename Ord<a>::pertain;

		template<typename = std::enable_if_t<pertain::value>>
		static Ordering compare(Maybe<a> one, Maybe<a> other)
		{
			if (isJust(one))
			{
				if (isJust(other))return Ord<a>::compare(fromJust(one), fromJust(other));
				return Ordering::GT;
			}
			if (isNothing(other))return Ordering::EQ;
			return Ordering::LT;
		}
	};

	//Maybe Show typeclass implenmentation
	template<typename a>
	struct Show<Maybe<a>>
	{
		using pertain = typename Show<a>::pertain;

		template<typename = std::enable_if_t<pertain::value>>
		static std::string show(const Maybe<a>& value)
		{
			if (isNothing(value)) return "Nothing";
			return "Just " + Show<a>::show(fromJust(value));
		}
	};

	//Maybe Functor typeclass implenmentation
	template<>
	struct Functor<Maybe>
	{
		using pertain = std::true_type;

		template<typename f, typename = std::enable_if_t<is_function<f>::value>>
		static Maybe<applied_type<f>> fmap(const f& func, const Maybe<head_parameter<f>>& ma)
		{
			if (isNothing(ma)) return Nothing();
			return Just<applied_type<f>>{func << fromJust(ma)};
		}
	};

	//Maybe Applicative typeclass implenmentation
	template<>
	struct Applicative<Maybe>
	{
		using pertain = std::true_type;

		template<typename a>
		static Maybe<a> pure(const a& value) { return Maybe<a>(value); }

		template<typename f, typename = std::enable_if_t<is_function<f>::value>>
		static Maybe<applied_type<f>> sequence(const Maybe<f>& mfunc, const Maybe<head_parameter<f>>& ma)
		{
			if (isNothing(mfunc)) return Nothing();
			return Functor<Maybe>::fmap(fromJust(mfunc), ma);
		}
	};

	//Maybe Alternative typeclass implenmentation
	template<>
	struct Alternative<Maybe>
	{
		using pertain = std::true_type;

		template<typename a>
		static Maybe<a> empty() { return Nothing(); }

		template<typename a>
		static Maybe<a> alter(const Maybe<a>& p, const Maybe<a>& q) { if (isJust(p)) return p; return q; }
	};

	//Maybe Monad typeclass implenmentation
	template<>
	struct Monad<Maybe>
	{
		using pertain = std::true_type;

		template<typename a>
		static Maybe<a> pure(const a& value) { return Applicative<Maybe>::pure<a>(value); }

		template<typename f, typename = std::enable_if_t<is_function<f>::value>>
		static Maybe<monadic_applied_type<f>> sequence(const Maybe<last_parameter<f>>& ma, const Maybe<f>& mfunc)
		{
			if (isNothing(ma) || isNothing(mfunc)) return Nothing();
			return Just<monadic_applied_type<f>>{fromJust(ma) >>= fromJust(mfunc)};
		}

		template<typename a, typename b>
		static Maybe<b> compose(const Maybe<a>& p, const Maybe<b>& q) { return Alternative<Maybe>::alter(p, q); }
	};

	//Show typeclass common implenmentations

	template<>
	struct Show<NA>
	{
		using pertain = std::true_type;

		static std::string show(const NA& value) { return "()"; }
	};

	template<typename a>
	struct Show<Tuple<a>>
	{
		using pertain = typename Show<a>::pertain;

		template<typename = std::enable_if_t<pertain::value>>
		static std::string show(const Tuple<a>& value) { return "(" + content(value) + ")"; }

		template<typename = std::enable_if_t<pertain::value>>
		static std::string content(Tuple<a> value) { return Show<a>::show(std::get<0>(value)); }
	};

	template<typename a, typename b>
	struct Show<Pair<a, b>>
	{
		using pertain = details::are_show<a, b>;

		template<typename = std::enable_if_t<pertain::value>>
		static std::string show(const Pair<a, b>& value) { return "(" + Show<a>::show(value.first) + "," + Show<b>::show(value.second) + ")"; }
	};

	template<typename a, typename ...as>
	struct Show<Tuple<a, as...>>
	{
		using pertain = details::are_show<a, as...>;

		template<typename = std::enable_if_t<pertain::value>>
		static std::string show(const Tuple<a, as...>& value) { return "(" + content(value) + ")"; }

		template<typename = std::enable_if_t<pertain::value>>
		static std::string content(Tuple<a, as...> value)
		{
			auto first = Show<a>::show(std::get<0>(value));
			return first + "," + Show<Tuple<as...>>::content(details::tail(std::move(value)));
		}
	};

	template<>
	struct Show<std::string>
	{
		using pertain = std::true_type;
		static std::string show(const std::string& value) { return value; }
	};

	template<typename a, typename b, typename ...rest>
	struct Show<variant<a, b, rest...>>
	{
		using pertain = details::are_show<a, b, rest...>;
		using V = variant<a, b, rest...>;
	private:

		template<size_t I, typename = std::enable_if_t<I == TMP::length<V>::value>>
		static std::string show_impl(const V& value)
		{
			throw std::exception("error: type mismatch.");
		}

		template<size_t I, typename = std::enable_if_t<I != TMP::length<V>::value>, size_t = 0>
		static std::string show_impl(const V& value)
		{
			using T = typename TMP::index<V, I>::type;
			if (variant_traits<V>::is_of<T>(value))
				return util::type<T>::infer() + " " + Show<T>::show(variant_traits<V>::get<T>(value));
			return show_impl<I + 1>(value);
		}

	public:

		template<typename = std::enable_if_t<pertain::value>>
		static std::string show(const V& value) { return show_impl<0>(value); }
	};

	template<typename a>
	struct Show<List<a>>
	{
		using pertain = typename Show<a>::pertain;

		template<typename = std::enable_if_t<pertain::value>>
		static std::string show(const List<a>& value)
		{
			if (value.size() == 0) return "[]";
			if (value.size() == 1)
				return "[" + Show<a>::show(value.front()) + "]";
			std::string buffer = "[";
			auto it = value.cbegin();
			buffer += Show<a>::show(*it);
			it++;
			while (it != value.cend())
			{
				buffer += "," + Show<a>::show(*it);
				it++;
			}
			return buffer + "]";
		}
	};

	//pattern matching syntax
	template<typename r>
	struct of
	{
		template<typename ...as>
		struct pattern
		{
			using tuples = std::tuple<as...>;

			pattern(as...arg) :var_(std::make_tuple(arg...)), r_() {}

			template<typename...bs>
			pattern<as...>& match(r result)
			{
				static_assert(details::are_legal<TMP::List<as...>, TMP::List<bs...>>::value, "error: type mismatch.");
				if (isJust(r_)) return *this;
				if (std::apply(details::pattern<as...>::match<bs...>, var_))
					r_ = result;
				return *this;
			}

			template<typename...bs>
			pattern<as...>& match(const Function<r, bs...>& f)
			{
				static_assert(details::are_legal<TMP::List<as...>, TMP::List<bs...>>::value, "error: type mismatch.");
				if (isJust(r_)) return *this;
				if (std::apply(details::pattern<as...>::match<bs...>, var_))
					r_ = std::apply(f, std::apply(details::pattern<as...>::gets<bs...>, std::move(var_)));
				return *this;
			}

			operator r()
			{
				if (isJust(r_)) return fromJust(std::move(r_));
				throw std::exception("error: pattern exhausted without match.");
			}

		private:
			tuples var_;
			Maybe<r> r_;
		};

		template<>
		struct pattern<>;

		template<typename a>
		struct pattern<a>
		{
			pattern(a var) :var_(var), r_() { static_assert(variant_traits<a>::possess::value, "type has no variant traits."); }

			template<typename b>
			pattern& match(r result)
			{
				static_assert(variant_traits<a>::elem<b>::value, "error: type mismatch.");
				if (isJust(r_)) return *this;
				if (variant_traits<a>::is_of<b>(var_))
					r_ = result;
				return *this;
			}

			template<typename b>
			pattern& match(const Function<r, b>& f)
			{
				static_assert(variant_traits<a>::elem<b>::value, "error: type mismatch.");
				if (isJust(r_)) return *this;
				if (variant_traits<a>::is_of<b>(var_))
					r_ = result;
				return *this;
			}

			template<template<typename> typename ctor, typename = std::enable_if_t<variant_traits<a>::has_default::value>>
			pattern& match(r result)
			{
				using b = ctor<typename variant_traits<a>::default_type>;
				return match<b>(result);
			}

			template<template<typename> typename ctor, typename = std::enable_if_t<variant_traits<a>::has_default::value>>
			pattern& match(const Function<r, ctor<typename variant_traits<a>::default_type>>& f)
			{
				using b = ctor<typename variant_traits<a>::default_type>;
				return match<b>(f);
			}

			operator r()
			{
				if (isJust(r_)) return fromJust(std::move(r_));
				throw std::exception("error: pattern exhausted without match.");
			}


		private:
			a var_;
			Maybe<r> r_;
		};

	};
}

//type inference for Maybes
namespace util
{
	template<typename a>
	struct type<fcl::Maybe<a>> { static std::string infer() { return "Maybe " + type<a>::infer(); } };

	template<typename a>
	struct type<fcl::Just<a>> { static std::string infer() { return "Just " + type<a>::infer(); } };

	template<>
	struct type<fcl::Nothing> { static std::string infer() { return "Nothing"; } };
}

#endif