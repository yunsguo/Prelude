#include "JSON.h"


#include <iostream>

using namespace JSON;
using namespace fcl;

fcl::Reaction<Null> JSON::null(std::string inp)
{
	const static auto M =
		fcl::string("null") >>
		fcl::Monad<fcl::Parser>::pure<Null>(Null())
		;
	return fcl::parse(M, inp);
}

fcl::Reaction<True> JSON::bool_true(std::string inp)
{
	const static auto M =
		fcl::string("true") >>
		fcl::Monad<fcl::Parser>::pure<True>(True())
		;
	return fcl::parse(M, inp);
}

fcl::Reaction<False> JSON::bool_false(std::string inp)
{
	const static auto M =
		fcl::string("false") >>
		fcl::Monad<fcl::Parser>::pure<False>(False())
		;
	return fcl::parse(M, inp);
}

fcl::Reaction<std::string> JSON::integer(std::string inp)
{
	const static auto M =
		maybe_one(fcl::character('-')) >>=
		(
			fcl::string("0") || (
				Parser<char>(digit19) >>=
				any(digit) >>=
				Monad<Parser>::pure<Function<std::string, char, std::string>>(cons_str)
				)) >>=
		Monad<Parser>::pure<Function<std::string, std::string, std::string>>(concat_str);

	return fcl::parse(M, inp);
}

fcl::Reaction<std::string> JSON::decimal(std::string inp)
{
	const static auto M =
		(fcl::character('.') >>=
			some(digit) >>=
			Monad<Parser>::pure<Function<std::string, char, std::string>>(cons_str)) ||
		Monad<Parser>::pure<std::string>("");

	return fcl::parse(M, inp);
}

fcl::Reaction<std::string> JSON::exponent(std::string inp)
{
	const static auto M =
		((fcl::character('e') || fcl::character('E')) >>=
			maybe_one(sat([](char c)->bool {return c == '+' || c == '-'; })) >>=
			some(digit) >>=
			Monad<Parser>::pure<Function<std::string, char, std::string, std::string>>
			([](char e, std::string p, std::string q)->std::string { return e + p + q; })) ||
		Monad<Parser>::pure<std::string>("");

	return fcl::parse(M, inp);
}

fcl::Reaction<Number> JSON::number(std::string inp)
{
	const static auto M =
		Parser<std::string>(JSON::integer) >>=
		Parser<std::string>(JSON::decimal) >>=
		Parser<std::string>(JSON::exponent) >>=
		Monad<Parser>::pure<Function<std::string, std::string, std::string, std::string>>
		([](std::string s1, std::string s2, std::string s3)->std::string {return s1 + s2 + s3; });

	auto reaction = parse(M, inp);
	if (isNothing(reaction)) return Nothing();
	auto pair = fromJust(reaction);
	return std::make_pair(Number{ std::stof(pair.first) }, std::move(pair.second));
}

char32_t JSON::char_to_uni(char c) { return c; }

char32_t JSON::unicode(char c1, char c2, char c3, char c4) { return strtoul("" + c1 + c2 + c3 + c4, 0, 16); }

fcl::Reaction<char32_t> JSON::character(std::string inp)
{
	const static Parser<char> any_except = fcl::sat
	([](char c)->bool {return c != '\"' && c != '\\' && iscntrl(c) == 0; });

	const static Parser<char> special = fcl::sat
	([](char c)->bool {return c == '\"' || c == '\\' || c == '/' || c == '\b' || c == '\f' || c == '\n' || c == '\r' || c == '\t'; });

	const static auto C2U = Monad<Parser>::pure<Function<char32_t, char>>(char_to_uni);

	const static Parser<char> hex = fcl::hexdecimal;

	const static auto UNI = Monad<Parser>::pure<Function<char32_t, char, char, char, char>>(unicode);

	const static auto M =
		(any_except >>=
			C2U) ||
			(fcl::character('\\') >>
		((special >>=
			C2U) ||
			(fcl::character('u') >>
				hex >>=
				hex >>=
				hex >>=
				hex >>=
				UNI)));

	return parse(M, inp);
}

fcl::Reaction<String> JSON::string(std::string inp)
{
	const static auto M =
		fcl::character('\"') >>
		fcl::any<char32_t>(JSON::character) >>=
		fcl::character('\"') >>
		Monad<Parser>::pure<Function<fcl::List<char32_t>, fcl::List<char32_t>>>(fcl::id<fcl::List<char32_t>>);
	auto reaction = parse(M, inp);
	if (isNothing(reaction)) return Nothing();
	auto pair = fromJust(reaction);
	return std::make_pair<String, std::string>(String{ std::move(pair.first) }, std::move(pair.second));
}

fcl::Reaction<Value> JSON::value(std::string inp)
{
	const static auto M =
		(Parser<String>(JSON::string) >>=
			Monad<Parser>::pure<Function<Value, String>>(JSON::boxing<String>))
		||
		(Parser<Number>(JSON::number) >>=
			Monad<Parser>::pure<Function<Value, Number>>(JSON::boxing<Number>))
		||
		(Parser<Object>(JSON::object) >>=
			Monad<Parser>::pure<Function<Value, Object>>(JSON::boxing<Object>))
		||
		(Parser<Array>(JSON::array) >>=
			Monad<Parser>::pure<Function<Value, Array>>(JSON::boxing<Array>))
		||
		(Parser<True>(JSON::bool_true) >>=
			Monad<Parser>::pure<Function<Value, True>>(JSON::boxing<True>))
		||
		(Parser<False>(JSON::bool_false) >>=
			Monad<Parser>::pure<Function<Value, False>>(JSON::boxing<False>))
		||
		(Parser<Null>(JSON::null) >>=
			Monad<Parser>::pure<Function<Value, Null>>(JSON::boxing<Null>));

	return parse(M, inp);
}

fcl::Reaction<Array> JSON::array(std::string inp)
{
	const static auto M =
		fcl::character('[') >>
		fcl::any<Value>(
			Parser<Value>(JSON::value) >>=
			fcl::character(',') >>
			Monad<Parser>::pure<Function<Value, Value>>(fcl::id<Value>)
			) >>=
		fcl::character(']') >>
		Monad<Parser>::pure<Function<fcl::List<Value>, fcl::List<Value>>>(fcl::id<fcl::List<Value>>);
	auto reaction = parse(M, inp);
	if (isNothing(reaction)) return Nothing();
	auto pair = fromJust(reaction);

	return std::make_pair<Array, std::string>(Array{ std::move(pair.first) }, std::move(pair.second));
}

Pair<String, Value> JSON::pair(String str, Value val)
{
	return std::make_pair<String, Value>(std::move(str), std::move(val));
}

fcl::Reaction<Object> JSON::object(std::string inp)
{
	const static auto M =
		fcl::character('{') >>
		any<Pair<String, Value>>(
			Parser<String>(JSON::string) >>=
			fcl::character(':') >>
			Parser<Value>(JSON::value) >>=
			fcl::character(',') >>
			Monad<Parser>::pure<Function<Pair<String, Value>, String, Value>>(JSON::pair)
			) >>=
		fcl::character('}') >>
		Monad<Parser>::pure<Function<fcl::List<Pair<String, Value>>, fcl::List<Pair<String, Value>>>>(fcl::id<fcl::List<Pair<String, Value>>>);

	auto reaction = parse(M, inp);
	if (isNothing(reaction)) return Nothing();
	auto pair = fromJust(reaction);

	return std::make_pair<Object, std::string>(Object{std::move(pair.first)},std::move(pair.second));
}

template<>
struct fcl::Show<JSON::True>
{
	using pertain = std::true_type;
	inline static std::string show(const JSON::True& value) { return "true"; }
};

template<>
struct fcl::Show<JSON::False>
{
	using pertain = std::true_type;
	inline static std::string show(const JSON::False& value) { return "false"; }
};

template<>
struct fcl::Show<JSON::Null>
{
	using pertain = std::true_type;
	inline static std::string show(const JSON::Null& value) { return "null"; }
};

template<>
struct fcl::Show<JSON::String>
{
	using pertain = std::true_type;
	inline static std::string show(const JSON::String& value) { return fcl::Show<fcl::List<char32_t>>::show(value.value); }
};

template<>
struct fcl::Show<JSON::Number>
{
	using pertain = std::true_type;
	inline static std::string show(const JSON::Number& value) { return std::to_string(value.value); }
};

//template<>
//struct fcl::Show<JSON::Value>
//{
//	using pertain = std::true_type;
//	inline static std::string show(const JSON::Value& value) { return fcl::Show<JSON::VD>::show(value.value); }
//};

//template<>
//struct fcl::Show<JSON::Array>
//{
//	using pertain = std::true_type;
//	inline static std::string show(const JSON::Array& value) { return fcl::Show<fcl::List<Value>>::show(value.value); }
//};
//
//template<>
//struct fcl::Show<JSON::Object>
//{
//	using pertain = std::true_type;
//	inline static std::string show(const JSON::Object& value) { return fcl::Show<fcl::List<Pair<String, Value>>>::show(value.value); }
//};


template<typename a>
void display_parse(Parser<a> p, std::string str)
{
	std::cout << "parse \"" << str << "\": " << parse(p, str) << std::endl;
}


int main()
{
	display_parse<False>(bool_false, "false");
	display_parse<True>(bool_true, "true");
	display_parse<Null>(null, "null");
	display_parse<Number>(number, "686.97 365.24");
	display_parse<Number>(number, "null");
	display_parse<String>(JSON::string, "\"null\"");
}