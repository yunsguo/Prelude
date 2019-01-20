/*
*   parsing.h:
*	previously Hutton.h
*   A port of Hutton's parsing library
*	included in solution directory
*   Language: C++, Visual Studio 2017
*   Platform: Windows 10 Pro
*   Application: recreational
*   Author: Yunsheng Guo, yguo125@syr.edu
*/


/*
*
*   Package Operations:
*	utilize monad operations to create a general parsing library
*	by combining Parser monad
*
*   Public Interface:
*	check Hutton's parsing library for all methods
*
*   Build Process:
*   requires prelude.h
*
*   Maintenance History:
*   July 18
*   First draft
*   late July
*   decided that in order to achieve monadic sequence, reverse apply function order has to be achieved
*	August 20
*	refactor according to other dependents
*	August 25
*	final review for publish
*
*
*/

#pragma once
#ifndef _PARSER_
#define _PARSER_

#include "prelude.h"

namespace fcl
{

	template<typename a>
	using Reaction = Maybe<pair<a, std::string>>;

	template<typename a>
	struct Parser;

	template<typename a>
	Reaction<a> parse(const Parser<a>&, std::string);

	template<typename a>
	struct Parser
	{
		template<typename l, typename = std::enable_if_t<std::is_convertible<l, function<Reaction<a>, std::string>>::value>>
		Parser(l lambda) :P(lambda) {}
		friend Reaction<a> parse<>(const Parser<a>&, std::string);
	private:
		function<Reaction<a>, std::string> P;
	};

	template<typename a>
	Reaction<a> parse(const Parser<a>& p, std::string str) { return p.P(str); }

	template<typename a>
	Reaction<a> failure(std::string inp)
	{
		const static Nothing nothing = Nothing();
		return nothing;
	}

	template<typename a>
	Reaction<a> reaction(a&& value, std::string&& inp)
	{
		return std::make_pair<a, std::string>(std::forward<a>(value), std::forward<std::string>(inp));
	}

	template<>
	struct Injector<Parser>
	{
		using pertain = std::true_type;

		template<typename a>
		static Parser<a> pure(a&& value)
		{
			const static function<Reaction<a>, a, std::string> pure_impl =
				[](a val, std::string inp)->Reaction<a>
			{
				return reaction(std::move(val), std::move(inp));
			};

			return pure_impl << std::forward<a>(value);
		}
	};

	template<>
	struct Functor<Parser>
	{
		using pertain = std::true_type;

		template<typename f, typename = std::enable_if_t<is_function<f>::value>>
		static Parser<applied_type<f>> fmap(f&& f_, Parser<head_parameter<f>>&& pa)
		{
			const static function<Reaction<applied_type<f>>, f, Parser<head_parameter<f>>, std::string> fmap_impl =
				[](f func, Parser<head_parameter<f>> pa, std::string inp)->Reaction<applied_type<f>>
			{
				auto r = parse(pa, std::move(inp));
				if (isNothing(r)) return Nothing();
				auto pair = fromJust(std::move(r));
				return reaction(function_traits<f>::apply(func, std::move(pair.first)), std::move(pair.second));
			};

			return fmap_impl << std::forward<f>(f_) << std::forward<Parser<head_parameter<f>>>(pa);
		}

		template<typename f, typename = std::enable_if_t<is_function<f>::value>>
		static Parser<monadic_applied_type<f>> monadic_fmap(const f& f_, Parser<last_parameter<f>>&& pa)
		{
			const static function<Reaction<monadic_applied_type<f>>, f, Parser<last_parameter<f>>, std::string> fmap_impl =
				[](f func, Parser<last_parameter<f>> pa, std::string inp)->Reaction<monadic_applied_type<f>>
			{
				auto r = parse(pa, std::move(inp));
				if (isNothing(r)) return Nothing();
				auto pair = fromJust(std::move(r));
				return reaction(function_traits<f>::monadic_apply(func,std::move(pair.first)), std::move(pair.second));
			};

			return fmap_impl << f_ << std::forward<Parser<last_parameter<f>>>(pa);
		}
	};

	template<>
	struct Alternative<Parser>
	{
		using pertain = std::true_type;

		template<typename a>
		static Parser<a> empty() { return failure<a>; }

		template<typename a>
		static Parser<a> alter(Parser<a>&& p, Parser<a>&& q)
		{
			const static function<Reaction<a>, Parser<a>, Parser<a>, std::string> alter_impl =
				[](Parser<a> p1, Parser<a> q1, std::string inp)->Reaction<a>
			{
				auto r = parse(p1, inp);
				if (isJust(r)) return r;
				return parse(q1, std::move(inp));
			};

			return alter_impl << std::forward<Parser<a>>(p) << std::forward<Parser<a>>(q);
		}
	};

	template<>
	struct Monad<Parser>
	{
		using pertain = std::true_type;

		template<typename f, typename = std::enable_if_t<is_function<f>::value>>
		static Parser<monadic_applied_type<f>> sequence(Parser<f>&& pf, Parser<last_parameter<f>>&& pa)
		{
			const static function<Reaction<monadic_applied_type<f>>, Parser<last_parameter<f>>, Parser<f>, std::string> seq_impl =
				[](Parser<last_parameter<f>> pa1, Parser<f> pf1, std::string inp)->Reaction<monadic_applied_type<f>>
			{
				auto ra = parse(pa1, std::move(inp));
				if (isNothing(ra)) return Nothing();
				auto paira = fromJust(std::move(ra));
				auto rf = parse(pf1, std::move(paira.second));
				if (isNothing(rf)) return Nothing();
				auto pairf = fromJust(std::move(rf));
				return reaction(function_traits<f>::monadic_apply(pairf.first, std::move(paira.first)), std::move(pairf.second));
			};

			return seq_impl << std::forward<Parser<last_parameter<f>>>(pa) << std::forward<Parser<f>>(pf);
		}

		template<typename a, typename b>
		static Parser<b> compose(Parser<a>&& p, Parser<b>&& q)
		{
			const static function<Reaction<b>, Parser<a>, Parser<b>, std::string> compose_impl =
				[](Parser<a> p1, Parser<b> q1, std::string inp)->Reaction<b>
			{
				auto r = parse(p1, std::move(inp));
				if (isNothing(r)) return Nothing();
				return parse(q1, fromJust(std::move(r)).second);
			};

			return compose_impl << std::forward<Parser<a>>(p) << std::forward<Parser<b>>(q);
		}
	};

	Reaction<char> item(std::string inp);

	Parser<char> sat(function<bool, char> p);

	Reaction<char> digit(std::string inp);

	Reaction<char> digit19(std::string inp);

	Reaction<char> hexdecimal(std::string inp);

	Reaction<char> lower(std::string inp);

	Reaction<char> upper(std::string inp);

	Reaction<char> letter(std::string inp);

	Reaction<char> alphanumeric(std::string inp);

	Parser<char> character(char c);

	std::string one_char_str(char c);

	Parser<std::string> string(std::string str);

	Reaction<std::string> ident(std::string inp);

	Reaction<int> nat(std::string inp);

	Reaction<int> inte(std::string inp);

	Reaction<na> space(std::string inp);

	Reaction<std::string> identifier(std::string inp);

	Reaction<int> natural(std::string inp);

	Reaction<int> integer(std::string inp);

	Parser<std::string> symbol(std::string str);

	std::string cons_str(char c, std::string str);

	pair<char, std::string> uncons_str(std::string str);

	std::string concat_str(std::string, std::string);

	Parser<std::string> maybe_one(Parser<char> p);

	Parser<std::string> any(Parser<char> p);

	Parser<std::string> some(Parser<char> p);

	template<typename a>
	Parser<list<a>> maybe_one(Parser<a> p);

	template<typename a>
	Parser<list<a>> any(Parser<a> p);

	template<typename a>
	Parser<list<a>> some(Parser<a> p);

	template<typename a>
	Parser<a> token(Parser<a> p)
	{
		const static auto Fid = function<a, a>(id<a>);
		const static auto Fspace = Parser<na>(space);
		return
			Fspace >>
			p >>=
			Fspace >>
			Fid;
	}

#ifdef PARSER_CPP

	int add(int a, int b) { return a + b; }

	int mul(int a, int b) { return a * b; }

	Reaction<int> expr(std::string inp);

	Reaction<int> term(std::string inp);

	Reaction<int> factor(std::string inp);

	int digitToInt(char c) { return c - '0'; }

	int eval(std::string inp);

#endif

	//implenmentation

	template<typename a>
	inline Parser<list<a>> maybe_one(Parser<a> p)
	{
		const static function<Reaction<list<a>>, Parser<a>, std::string> maybe_one_impl =
			[](Parser<a> p1, std::string inp)->Reaction<list<a>>
		{
			auto r = parse(p1, inp);
			if (isNothing(r)) return reaction(list<a>(), std::move(inp));
			auto pair = fromJust(r);
			return reaction(list<a>{pair.first}, std::move(pair.second));
		};

		return maybe_one_impl << p;
	}

	template<typename a>
	inline Parser<list<a>> any(Parser<a> p)
	{
		const static function<Reaction<list<a>>, Parser<a>, std::string> any_impl =
			[](Parser<a> p, std::string inp) ->Reaction<list<a>>
		{
			auto r = parse(p, inp);
			if (isNothing(r))
				return reaction(list<a>(), std::move(inp));
			auto pair = fromJust(r);
			list<a> ans;
			std::string inpn;
			while (true)
			{
				ans.push_back(std::move(pair.first));
				inpn = std::move(pair.second);
				r = parse(p, inpn);
				if (isNothing(r)) return reaction(std::move(ans), std::move(inpn));
				pair = fromJust(r);
			}
		};
		return any_impl << p;
	}

	template<typename a>
	inline Parser<list<a>> some(Parser<a> p)
	{
		const static auto Fcons = function<list<a>, a, list<a>>(cons<a>);
		return
			p >>=
			any<a>(p) >>=
			Fcons;
	}
}

#endif