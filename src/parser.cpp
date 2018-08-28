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
	if (inp.length() == 0)return Nothing();
	return uncons_str(inp);
}

Reaction<char> sat_impl(Function<bool, char> p, std::string inp)
{
	if (inp.length() == 0) return Nothing();
	auto pair = uncons_str(inp);
	if (p(pair.first)) return pair;
	return Nothing();
}

Parser<char> fcl::sat(Function<bool, char> p)
{
	return Function<Reaction<char>, Function<bool, char>, std::string>(sat_impl) << p;
}

Reaction<char> fcl::digit(std::string inp) { return parse(sat([](char c)->bool {return isdigit(c); }), inp); }

Reaction<char> fcl::lower(std::string inp) { return parse(sat([](char c)->bool {return islower(c); }), inp); }

Reaction<char> fcl::upper(std::string inp) { return parse(sat([](char c)->bool {return isupper(c); }), inp); }

Reaction<char> fcl::letter(std::string inp) { return parse(sat([](char c)->bool {return isalpha(c); }), inp); }

Reaction<char> fcl::alphanumeric(std::string inp) { return parse(sat([](char c)->bool {return isalnum(c); }), inp); }

bool char_equal(char a, char b) { return a == b; }

Parser<char> fcl::character(char c) { return sat(Function<bool, char, char>(char_equal) << c); }

Parser<std::string> fcl::string(std::string str)
{
	if (str.length() == 0) return Monad<Parser>::pure<std::string>("");
	auto pair = uncons_str(str);
	return
		Parser<char>(character(pair.first)) >>=
		Parser<std::string>(string(pair.second)) >>=
		Monad<Parser>::pure(Function<std::string, char, std::string>(cons_str));
}

Reaction<std::string> fcl::ident(std::string inp)
{
	return parse(
		Parser<char>(lower) >>=
		any(Parser<char>(alphanumeric)) >>=
		Monad<Parser>::pure(Function<std::string, char, std::string>(cons_str))
		, inp
	);
}

int fcl::read_int(std::string str) { return std::stoi(str); }

Reaction<int> fcl::nat(std::string inp)
{
	return parse(
		some(Parser<char>(digit)) >>=
		Monad<Parser>::pure(Function<int, std::string>(read_int))
		, inp
	);
}

Reaction<int> fcl::inte(std::string inp)
{
	return parse(
		(character('-') >>
			Parser<int>(nat) >>=
			Monad<Parser>::pure<Function<int, int>>([](int n) { return -1 * n; }))
		||
		Parser<int>(nat)
		, inp
	);
}

bool fcl::isSpace(char c)
{
	return isspace(c);
}

Reaction<NA> fcl::space(std::string inp)
{
	const static auto MNA = Monad<Parser>::pure<NA>(NA());
	return parse(
		any(sat(isSpace)) >>
		MNA
		, inp);
}

Reaction<std::string> fcl::identifier(std::string inp) { return parse(token(Parser<std::string>(ident)), inp); }

Reaction<int> fcl::natural(std::string inp) { return parse(token(Parser<int>(nat)), inp); }

Reaction<int> fcl::integer(std::string inp) { return parse(token(Parser<int>(inte)), inp); }

Parser<std::string> fcl::symbol(std::string str) { return token(Parser<std::string>(string(str))); }

std::string fcl::cons_str(char c, std::string str) { return c + str; }

Pair<char, std::string> fcl::uncons_str(std::string str)
{
	return std::make_pair<char, std::string>(std::move(str[0]), str.substr(1, -1));
}


#ifdef PARSER_CPP


Reaction<int> fcl::expr(std::string inp)
{
	return parse(
		(Parser<int>(term) >>=
			character('+') >>
			Parser<int>(expr) >>=
			Monad<Parser>::pure<Function<int, int, int>>(add))
		||
		Parser<int>(term)
		, inp);
}

Reaction<int> fcl::term(std::string inp)
{
	return parse(
		(Parser<int>(factor) >>=
			character('*') >>
			Parser<int>(term) >>=
			Monad<Parser>::pure<Function<int, int, int>>(mul))
		||
		Parser<int>(factor)
		, inp);
}

Reaction<int> fcl::factor(std::string inp)
{
	return parse(
		(Parser<char>(digit) >>=
			Monad<Parser>::pure<Function<int, char>>(digitToInt))
		||
		(character('(') >>
			Parser<int>(expr) >>=
			character(')') >>
			Monad<Parser>::pure<Function<int, int>>(id<int>))
		, inp);
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