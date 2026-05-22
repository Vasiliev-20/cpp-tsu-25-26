#include "Scaner.h"
#include <iostream>
#include <string>
#include <map>

Token::Token(LexemType type) : type_(type), value_(0), str_("") {}
Token::Token(int value) : type_(LexemType::num), value_(value), str_("") {}
Token::Token(LexemType type, const std::string& str) : type_(type), value_(0), str_(str) {}
Token::Token(char c) : type_(LexemType::chr), value_((int)c), str_("") {}
Token::Token(int value, const std::string& str) : type_(LexemType::error), value_(value), str_(str) {}

std::string Token::LexemTypeToString_(LexemType type) {
	switch (type) {
	case LexemType::num: return "num";
	case LexemType::chr: return "chr"; 
    case LexemType::str: return "str";
    case LexemType::id: return "id";

    case LexemType::lpar: return "lpar";
    case LexemType::rpar: return "rpar";
    case LexemType::lbrace: return "lbrace";
    case LexemType::rbrace: return "rbrace";
    case LexemType::lbracket: return "lbracket";
    case LexemType::rbracket: return "rbracket";

    case LexemType::semicolon: return "semicolon";
    case LexemType::comma: return "comma";
    case LexemType::colon: return "colon";

    case LexemType::opassign: return "opassign";
    case LexemType::opplus: return "opplus";
    case LexemType::opminus: return "opminus";
    case LexemType::opmult: return "opmult";
    case LexemType::opinc: return "opinc";
    case LexemType::opeq: return "opeq";
    case LexemType::opne: return "opne";
    case LexemType::oplt: return "oplt";
    case LexemType::opgt: return "opgt";
    case LexemType::ople: return "ople";
    case LexemType::opnot: return "opnot";
    case LexemType::opor: return "opor";
    case LexemType::opand: return "opand";

    case LexemType::kwint: return "kwint";
    case LexemType::kwchar: return "kwchar";
    case LexemType::kwif: return "kwif";
    case LexemType::kwelse: return "kwelse";
    case LexemType::kwswitch: return "kwswitch";
    case LexemType::kwcase: return "kwcase";
    case LexemType::kwdefault: return "kwdefault";
    case LexemType::kwwhile: return "kwwhile";
    case LexemType::kwfor: return "kwfor";
    case LexemType::kwreturn: return "kwreturn";
    case LexemType::kwin: return "kwin";
    case LexemType::kwout: return "kwout";

    case LexemType::eof: return "eof";
    case LexemType::error: return "error";
    }

    return "unknown";
}

void Token::print(std::ostream& stream) {
	stream << "[" << LexemTypeToString_(type_);
    switch (type_) {
        case LexemType::num:
            stream << ", " << value_;
            break;

        case LexemType::chr:
            stream << ", '" << (char)value_ << "'";
            break;

        case LexemType::id:
        case LexemType::str:
        case LexemType::error:
            stream << ", \"" << str_ << "\"";
            break;

        default:
            break;
    }
    stream << "]" << std::endl;
}

LexemType Token::type() {
	return type_;
}

int Token::value() {
	return value_;
}

std::string Token::str(){
	return str_;
}

std::map<char, LexemType> punctuation{ {'(', LexemType::lpar}, {')', LexemType::rpar}, {'{', LexemType::lbrace},
    {'}', LexemType::rbrace}, {'[', LexemType::lbracket}, {']', LexemType::rbracket}, {';', LexemType::semicolon},
    {',', LexemType::comma}, {':', LexemType::colon}
};

std::map<std::string, LexemType> keywords{ {"int", LexemType::kwint}, {"char", LexemType::kwchar}, {"if", LexemType::kwif},
    {"else", LexemType::kwelse}, {"switch", LexemType::kwswitch}, {"case", LexemType::kwcase}, {"default", LexemType::kwdefault}, {"while", LexemType::kwwhile},
    {"for", LexemType::kwfor}, {"return", LexemType::kwreturn}, {"in", LexemType::kwin}, {"out", LexemType::kwout}
};

std::map<int, std::string> errors{
    {1, "invalid character"},
    {2, "single character |"},
    {3, "single character &"},
    {4, "empty symbolic constant"},
    {5, "a character constant contains more than one character"},
    {6, "unclosed string constant"},
    {7, "unclosed symbolic constant"}
};

Scaner::Scaner(std::istream& stream) : stream_(stream) {}

Token Scaner::getNextLexem() {
    State state = State0;
    int value = 0;
    std::string buffer = "";

    for (;;) {
        char c = stream_.get();
        switch (state) {
            case State0:
                if (!stream_.good()) {
                    return Token(LexemType::eof);
                }
                else if (isspace(c)) {
                    continue;
                }
                else if (punctuation.count(c)) {
                    return Token(punctuation[c]);
                }
                else if (c == '-') {
                    return Token(LexemType::opminus);
                }
                else if (c == '>') {
                    return Token(LexemType::opgt);
                }
                else if (c == '*') {
                    return Token(LexemType::opmult);
                }
                else if (isdigit(c)) {
                    value = c - '0';
                    state = State1;
                    continue;
                }
                else if (isalpha(c) || c == '_') {
                    buffer.clear();
                    buffer = c;
                    state = State5;
                    continue;
                }
                else if (c == '!') {
                    state = State7;
                    continue;
                }
                else if (c == '\'') {
                    state = State2;
                    continue;
                }
                else if (c == '\"') {
                    buffer.clear();
                    state = State4;
                    continue;
                }
                else if (c == '=') {
                    state = State9;
                    continue;
                }
                else if (c == '<') {
                    state = State8;
                    continue;
                }
                else if (c == '+') {
                    state = State10;
                    continue;
                }
                else if (c == '|') {
                    state = State11;
                    continue;
                }
                else if (c == '&') {
                    state = State12;
                    continue;
                }
                else {
                    return Token(1, errors[1]);
                }
                break;

            case State1:
                if (!stream_.good()) {
                    return Token(value);
                }
                if (isdigit(c)) {
                    value = value * 10 + (c - '0');
                }
                else {
                    stream_.unget();
                    return Token(value);
                }
                break;

            case State2:
                if (!stream_.good()) {
                    return Token(7, errors[7]);
                }
                if (c == '\'') {
                    return Token(4, errors[4]);
                }
                else {
                    value = c;
                    state = State3;
                }
                break;
            
            case State3:
                if (!stream_.good()) {
                    return Token(7, errors[7]);
                }
                if (c == '\'') {
                    state = State0;
                    return Token((char)value);
                }
                else {
                    return Token(5, errors[5]);
                }
                
                break;

            case State4:
                if (!stream_.good()) {
                    return Token(6, errors[6]);
                } 
                else if (c == '\"') {
                    return Token(LexemType::str, buffer);
                }
                else {
                    buffer += c;
                    state = State4;
                }
                break;

            case State5:
                if (!stream_.good()) {
                    if (keywords.count(buffer)) {
                        return Token(keywords[buffer]);
                    }
                    return Token(LexemType::id, buffer);
                }
                if (isalnum(c) || c == '_') {
                    buffer += c;
                    continue;
                }
                else {
                    stream_.unget();
                    if (keywords.count(buffer)) {
                        return Token(keywords[buffer]);
                    }
                    return Token(LexemType::id, buffer);
                }
                break;

            case State7:
                if (!stream_.good()) {
                    return Token(LexemType::opnot);
                }
                if (c == '=') {
                    return Token(LexemType::opne);
                }
                else {
                    stream_.unget();
                    return Token(LexemType::opnot);
                }
                break;

            case State8:
                if (!stream_.good()) {
                    return Token(LexemType::oplt);
                }
                if (c == '=') {
                    return Token(LexemType::ople);
                }
                else {
                    stream_.unget();
                    return Token(LexemType::oplt);
                }
                break;

            case State9:
                if (!stream_.good()) {
                    return Token(LexemType::opassign);
                }
                if (c == '=') {
                    return Token(LexemType::opeq);
                }
                else {
                    stream_.unget();
                    return Token(LexemType::opassign);
                }
                break;

            case State10:
                if (!stream_.good()) {
                    return Token(LexemType::opplus);
                }
                if (c == '+') {
                    return Token(LexemType::opinc);
                }
                else {
                    stream_.unget();
                    return Token(LexemType::opplus);
                }
                break;

            case State11:
                if (!stream_.good()) {
                    return Token(2, errors[2]);
                }
                if (c == '|') {
                    return Token(LexemType::opor);
                }
                else {
                    return Token(2, errors[2]);
                }
                break;

            case State12:
                if (!stream_.good()) {
                    return Token(3, errors[3]);
                }
                if (c == '&') {
                    return Token(LexemType::opand);
                }
                else {
                    return Token(3, errors[3]);
                }

        }
    }
}
