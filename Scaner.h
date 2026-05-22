#pragma once
#include <iostream>
#include <string>
#include <map>

enum class LexemType {
	num, chr, str, id, lpar, rpar, lbrace, rbrace, lbracket, rbracket,
	semicolon, comma, colon, opassign, opplus, opminus, opmult, opinc, opeq, opne, oplt, opgt, ople,
	opnot, opor, opand, kwint, kwchar, kwif, kwelse, kwswitch, kwcase, kwdefault, kwwhile, kwfor, kwreturn,
	kwin, kwout, eof, error
};

enum State {
	State0, State1, State2, State3, State4, State5,
	State7, State8, State9, State10, State11, State12
};

class Token {
private:
	LexemType type_;
	int value_;
	std::string str_;

	std::string LexemTypeToString_(LexemType type);

public:
	Token(LexemType type);
	Token(int value);
	Token(LexemType type, const std::string& str);
	Token(char c);
	Token(int value, const std::string& str);

	std::string toStringLexem(LexemType type) {
		return LexemTypeToString_(type);
	}

	void print(std::ostream& stream);

	LexemType type();

	int value();

	std::string str();
};

class Scaner {
private:
	std::istream& stream_;

public:
	Scaner(std::istream& stream);

	Token getNextLexem();
};

extern std::map<char, LexemType> punctuation;
extern std::map<std::string, LexemType> keywords;
extern std::map<int, std::string> errors;
