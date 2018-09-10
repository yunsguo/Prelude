/*
* parser.cpp
* test stub for parser.h
* previously due to the nature of function and type templates
* all the dependent packages have no implenmetation in their cpp files
* meaning as long as the header is included, the cpp file is not needed in compilation
* this package is different. It's just a normal package with all concrete implenmentation in cpp files
* Yunsheng Guo yguo125@syr.edu
*/

#include "parser.h"

using namespace fcl;

Reaction<char> fcl::item(std::string inp)
{
	if (inp.length() == 0) return Nothing();
	return uncons_str(inp);
}

Parser<char> fcl::sat(Function<bool, char> p)
{
	const static Function<Reaction<char>, Function<bool, char>, std::string> sat_impl =
		[](Function<bool, char> p, std::string inp)->Reaction<char>
	{
		if (inp.length() == 0) return Nothing();
		auto pair = uncons_str(inp);
		if (p(pair.first)) return pair;
		return Nothing();
	};

	return sat_impl << p;
}

Reaction<char> fcl::digit(std::string inp)
{
	const static auto P = sat([](char c)->bool {return isdigit(c) != 0; });
	return parse(P, std::move(inp));
}

Reaction<char> fcl::digit19(std::string inp)
{
	const static auto P = sat([](char c)->bool {return isdigit(c) != 0 && c != '0'; });
	return parse(P, std::move(inp));
}

Reaction<char> fcl::hexdecimal(std::string inp)
{
	const static auto P = sat([](char c)->bool {return isxdigit(c) != 0; });
	return parse(P, std::move(inp));
}

Reaction<char> fcl::lower(std::string inp)
{
	const static auto P = sat([](char c)->bool {return islower(c) != 0; });
	return parse(P, std::move(inp));
}

Reaction<char> fcl::upper(std::string inp)
{
	const static auto P = sat([](char c)->bool {return isupper(c) != 0; });
	return parse(P, std::move(inp));
}

Reaction<char> fcl::letter(std::string inp)
{
	const static auto P = sat([](char c)->bool {return isalpha(c) != 0; });
	return parse(P, std::move(inp));
}

Reaction<char> fcl::alphanumeric(std::string inp)
{
	const static auto P = sat([](char c)->bool {return isalnum(c) != 0; });
	return parse(P, std::move(inp));
}

Parser<char> fcl::character(char c)
{
	const static auto char_equal = Function<bool, char, char>([](char a, char b)-> bool {return a == b; });
	return sat(char_equal << c);
}

std::string fcl::one_char_str(char c) { return std::string(1, c); }

Parser<std::string> fcl::string(std::string str)
{
	const static auto Mempty_str = Injector<Parser>::pure<std::string>("");
	if (str.length() == 0) return Mempty_str;

	const static auto Fcons = Function<std::string, char, std::string>(cons_str);
	auto pair = uncons_str(str);
	return
		character(pair.first) >>=
		string(pair.second) >>=
		Fcons;
}

Reaction<std::string> fcl::ident(std::string inp)
{
	const static auto P =
		lower >>=
		any(alphanumeric) >>=
		Function<std::string, char, std::string>(cons_str)
		;

	return parse(P, std::move(inp));
}

Reaction<int> fcl::nat(std::string inp)
{
	const static auto P =
		some(Parser<char>(digit)) >>=
		Function<int, std::string>([](std::string str)->int {return std::stoi(str); })
		;

	return parse(P, std::move(inp));
}

Reaction<int> fcl::inte(std::string inp)
{
	const static auto P =
		(
			character('-') >>
			Parser<int>(nat) >>=
			Function<int, int>([](int n) { return -1 * n; })
			)
		||
		Parser<int>(nat)
		;

	return parse(P, std::move(inp));
}

Reaction<NA> fcl::space(std::string inp)
{
	const static auto P =
		any(sat(Function<bool, char>([](char c)->bool { return isspace(c); }))) >>
		Injector<Parser>::pure<NA>(NA())
		;
	return parse(P, std::move(inp));
}

Reaction<std::string> fcl::identifier(std::string inp)
{
	const static auto P = token<std::string>(ident);
	return parse(P, std::move(inp));
}

Reaction<int> fcl::natural(std::string inp)
{
	const static auto P = token<int>(nat);
	return parse(P, std::move(inp));
}

Reaction<int> fcl::integer(std::string inp)
{
	const static auto P = token<int>(inte);
	return parse(P, std::move(inp));
}

Parser<std::string> fcl::symbol(std::string str) { return token<std::string>(string(str)); }

std::string fcl::cons_str(char c, std::string str) { return c + str; }

Pair<char, std::string> fcl::uncons_str(std::string str)
{
	return std::make_pair<char, std::string>(std::move(str[0]), str.substr(1, -1));
}

std::string fcl::concat_str(std::string a, std::string b) { return a + b; }

Parser<std::string> fcl::maybe_one(Parser<char> p)
{
	const static Function<Reaction<std::string>, Parser<char>, std::string> maybe_one_impl =
		[](Parser<char> p1, std::string inp)->Reaction<std::string>
	{
		auto r = parse(p1, inp);
		if (isNothing(r)) return reaction<std::string>("", std::move(inp));
		auto pair = fromJust(r);
		return reaction<std::string>(std::string(1, pair.first), std::move(pair.second));
	};

	return maybe_one_impl << p;
}

Parser<std::string> fcl::any(Parser<char> p)
{
	const static Function<Reaction<std::string>, Parser<char>, std::string> any_impl =
		[](Parser<char> p, std::string inp)->Reaction<std::string>
	{
		auto r = parse(p, inp);
		if (isNothing(r))
			return reaction<std::string>("", std::move(inp));
		auto pair = fromJust(r);
		std::string ans = "";
		std::string inpn = "";
		while (true)
		{
			ans += pair.first;
			inpn = std::move(pair.second);
			r = parse(p, inpn);
			if (isNothing(r)) return reaction<std::string>(std::move(ans), std::move(inpn));
			pair = fromJust(r);
		}
	};

	return any_impl << p;
}

Parser<std::string> fcl::some(Parser<char> p)
{
	const static auto Mcons = Function<std::string, char, std::string>(cons_str);
	return
		p >>=
		any(p) >>=
		Mcons;
}


#ifdef PARSER_CPP

Reaction<int> fcl::expr(std::string inp)
{
	const static auto P =
		(
			Parser<int>(term) >>=
			character('+') >>
			Parser<int>(expr) >>=
			Function<int, int, int>(add)
			)
		||
		Parser<int>(term)
		;

	return parse(P, std::move(inp));
}

Reaction<int> fcl::term(std::string inp)
{
	const static auto P =
		(Parser<int>(factor) >>=
			character('*') >>
			Parser<int>(term) >>=
			Function<int, int, int>(mul))
		||
		Parser<int>(factor)
		;

	return parse(P, std::move(inp));
}

Reaction<int> fcl::factor(std::string inp)
{
	const static auto P =
		(
			Parser<char>(digit) >>=
			Function<int, char>(digitToInt)
			)
		||
		(
			character('(') >>
			Parser<int>(expr) >>=
			character(')') >>
			Function<int, int>(id<int>)
			)
		;

	return parse(P, std::move(inp));
}

int fcl::eval(std::string inp)
{
	auto reaction = parse(Parser<int>(expr), inp);
	if (isNothing(reaction)) throw std::exception("parse failed");
	return fromJust(reaction).first;
}

void display_expr(std::string str) { std::cout << str << ": " << eval(str) << std::endl; }

template<typename a>
void display_parse(Parser<a> p, std::string str)
{
	std::cout << "parse " << str << ": " << parse(p, str) << std::endl;
}


int main()
{
	display_expr("1+2");
	display_expr("1+2+3");
	display_expr("1+2*3");
	display_expr("(1+2)*3");
}

#endif