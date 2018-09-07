#pragma once
#ifndef _JSON_
#define _JSON_

#include "parser.h"

namespace JSON
{
	using fcl::List;
	using fcl::Pair;

	struct True {};
	struct False {};
	struct Null {};

	struct String { List<char32_t> value; };
	struct Number { float value; };

	struct Array;
	struct Object;
	struct Value;

	struct Array { List<Value> value; };

	struct Object { List<Pair<String, Value>> value; };

	using VD = fcl::variant<String, Number, Object, Array, True, False, Null>;

	struct Value { VD value; };


	fcl::Reaction<Null> null(std::string inp);

	fcl::Reaction<True> bool_true(std::string inp);

	fcl::Reaction<False> bool_false(std::string inp);

	fcl::Reaction<std::string> integer(std::string inp);

	fcl::Reaction<std::string> decimal(std::string inp);

	fcl::Reaction<std::string> exponent(std::string inp);

	fcl::Reaction<Number> number(std::string inp);

	char32_t char_to_uni(char c);

	char32_t unicode(char c1, char c2, char c3, char c4);

	fcl::Reaction<char32_t> character(std::string inp);

	fcl::Reaction<String> string(std::string inp);

	template<typename a, typename = std::enable_if_t<fcl::variant_traits<VD>::elem<a>::value>>
	Value boxing(a var) { return Value{ VD(std::move(var)) }; }

	fcl::Reaction<Value> value(std::string inp);

	fcl::Reaction<Array> array(std::string inp);

	Pair<String, Value> pair(String str, Value val);

	fcl::Reaction<Object> object(std::string inp);
}

#endif
