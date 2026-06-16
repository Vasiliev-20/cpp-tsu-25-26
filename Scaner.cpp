#include "Scaner.h"
#include <cctype>   // для isspace, isdigit, isalpha, isalnum

// ---------- Реализация Token ----------

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
        default: return "unknown";
    }
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

LexemType Token::type() { return type_; }
int Token::value() { return value_; }
std::string Token::str() { return str_; }

// ---------- Глобальные словари (оставлены как есть) ----------

std::map<char, LexemType> punctuation{
    {'(', LexemType::lpar}, {')', LexemType::rpar},
    {'{', LexemType::lbrace}, {'}', LexemType::rbrace},
    {'[', LexemType::lbracket}, {']', LexemType::rbracket},
    {';', LexemType::semicolon}, {',', LexemType::comma},
    {':', LexemType::colon}
};

std::map<std::string, LexemType> keywords{
    {"int", LexemType::kwint}, {"char", LexemType::kwchar},
    {"if", LexemType::kwif}, {"else", LexemType::kwelse},
    {"switch", LexemType::kwswitch}, {"case", LexemType::kwcase},
    {"default", LexemType::kwdefault},
    {"while", LexemType::kwwhile}, {"for", LexemType::kwfor},
    {"return", LexemType::kwreturn},
    {"in", LexemType::kwin}, {"out", LexemType::kwout}
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

// ---------- Реализация Scaner ----------

Scaner::Scaner(std::istream& stream) : stream_(stream) {}

Token Scaner::getNextLexem() {
    State st = State0;          // раньше было state
    int val = 0;                // раньше value
    std::string buf = "";       // раньше buffer

    for (;;) {
        char ch = stream_.get();
        switch (st) {
            case State0:
                if (!stream_.good()) {
                    return Token(LexemType::eof);
                }
                if (isspace(ch)) {
                    continue;
                }
                if (punctuation.count(ch)) {
                    return Token(punctuation[ch]);
                }
                if (ch == '-') return Token(LexemType::opminus);
                if (ch == '>') return Token(LexemType::opgt);
                if (ch == '*') return Token(LexemType::opmult);
                if (isdigit(ch)) {
                    val = ch - '0';
                    st = State1;
                    continue;
                }
                if (isalpha(ch) || ch == '_') {
                    buf.clear();
                    buf = ch;
                    st = State5;
                    continue;
                }
                if (ch == '!') { st = State7; continue; }
                if (ch == '\'') { st = State2; continue; }
                if (ch == '\"') { buf.clear(); st = State4; continue; }
                if (ch == '=') { st = State9; continue; }
                if (ch == '<') { st = State8; continue; }
                if (ch == '+') { st = State10; continue; }
                if (ch == '|') { st = State11; continue; }
                if (ch == '&') { st = State12; continue; }
                return Token(1, errors[1]);

            case State1:
                if (!stream_.good()) {
                    return Token(val);
                }
                if (isdigit(ch)) {
                    val = val * 10 + (ch - '0');
                } else {
                    stream_.unget();
                    return Token(val);
                }
                break;

            case State2:
                if (!stream_.good()) {
                    return Token(7, errors[7]);
                }
                if (ch == '\'') {
                    return Token(4, errors[4]);
                } else {
                    val = ch;
                    st = State3;
                }
                break;

            case State3:
                if (!stream_.good()) {
                    return Token(7, errors[7]);
                }
                if (ch == '\'') {
                    st = State0;
                    return Token((char)val);
                } else {
                    return Token(5, errors[5]);
                }
                break;

            case State4:
                if (!stream_.good()) {
                    return Token(6, errors[6]);
                }
                if (ch == '\"') {
                    return Token(LexemType::str, buf);
                } else {
                    buf += ch;
                }
                break;

            case State5:
                if (!stream_.good()) {
                    if (keywords.count(buf)) return Token(keywords[buf]);
                    return Token(LexemType::id, buf);
                }
                if (isalnum(ch) || ch == '_') {
                    buf += ch;
                } else {
                    stream_.unget();
                    if (keywords.count(buf)) return Token(keywords[buf]);
                    return Token(LexemType::id, buf);
                }
                break;

            case State7:
                if (!stream_.good()) return Token(LexemType::opnot);
                if (ch == '=') return Token(LexemType::opne);
                stream_.unget();
                return Token(LexemType::opnot);

            case State8:
                if (!stream_.good()) return Token(LexemType::oplt);
                if (ch == '=') return Token(LexemType::ople);
                stream_.unget();
                return Token(LexemType::oplt);

            case State9:
                if (!stream_.good()) return Token(LexemType::opassign);
                if (ch == '=') return Token(LexemType::opeq);
                stream_.unget();
                return Token(LexemType::opassign);

            case State10:
                if (!stream_.good()) return Token(LexemType::opplus);
                if (ch == '+') return Token(LexemType::opinc);
                stream_.unget();
                return Token(LexemType::opplus);

            case State11:
                if (!stream_.good()) return Token(2, errors[2]);
                if (ch == '|') return Token(LexemType::opor);
                return Token(2, errors[2]);

            case State12:
                if (!stream_.good()) return Token(3, errors[3]);
                if (ch == '&') return Token(LexemType::opand);
                return Token(3, errors[3]);
        }
    }
}
