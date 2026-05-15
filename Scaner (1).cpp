#include "Scaner.h"

// тут я преобразую тип лексемы в строку для вывода - типа из LexemType::num в "num"
string typeToString(LexemType type) {
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
        case LexemType::kwwhile: return "kwwhile";
        case LexemType::kwfor: return "kwfor";
        case LexemType::kwreturn: return "kwreturn";
        case LexemType::kwin: return "kwin";
        case LexemType::kwout: return "kwout";
        case LexemType::eof: return "eof";
        case LexemType::error: return "error";
        default: return "unknown";  //  иначе
    }
}

// метод вывода токена - печатает его в поток в формате [тип, значение]
void Token::print(ostream &stream) {
    stream << "[" << typeToString(_type);  // сначала тип

    // для чисел  -  значение типа [num, 42]
    if (_type == LexemType::num) stream << ", " << _value;
    // для символов -  символ в апострофах типа [chr, 'a']
    else if (_type == LexemType::chr) stream << ", '" << (char)_value << "'";
    // для идентификаторов, строк и ошибок -  строка в кавычках
    else if (_type == LexemType::id || _type == LexemType::str || _type == LexemType::error)
        stream << ", \"" << _str << "\"";
    // для остальных типов
    stream << "]";
}

// конструктор сканера - заполнение таблицы знаков препинания + ключевых слов
Scanner::Scanner(istream& stream) : _stream(stream) {
    // заполняю таблицу знаков препинания - каждому символу свой тип лексемы
    _punctuation = {
        {'(', LexemType::lpar},
        {')', LexemType::rpar},
        {'{', LexemType::lbrace},
        {'}', LexemType::rbrace},
        {'[', LexemType::lbracket},
        {']', LexemType::rbracket},
        {';', LexemType::semicolon},
        {',', LexemType::comma},
        {':', LexemType::colon}
    };

    //таблица ключевых слов - каждому слову свой тип лексемы
    _keywords = {
        {"int", LexemType::kwint},
        {"char", LexemType::kwchar},
        {"if", LexemType::kwif},
        {"else", LexemType::kwelse},
        {"switch", LexemType::kwswitch},
        {"case", LexemType::kwcase},
        {"while", LexemType::kwwhile},
        {"for", LexemType::kwfor},
        {"return", LexemType::kwreturn},
        {"in", LexemType::kwin},
        {"out", LexemType::kwout}
    };
}

// получаю символ из потока - если конец файла вернет EOF
int Scanner::getChar() { return _stream.get(); }

// возвращаю символ обратно в поток - типа откатываюсь на один символ назад
void Scanner::ungetChar() { _stream.unget(); }

// проверка на пробельный символ - пробел, табуляция, перенос строки
bool Scanner::isWhiteSpace(int c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

// проверка на цифру от 0 до 9
bool Scanner::isDigit(int c) {
    return c >= '0' && c <= '9';
}

// проверка на букву английскую или подчеркивание
bool Scanner::isLetter(int c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

// главный метод - получаю следующий токен используя конечный автомат
Token Scanner::getNextToken() {
    _lexeme.clear();
    int state = 0;
    while (true) {
        int c = getChar();

        // если конец файла - обработка в зависимости от текущего состояния
        if (c == EOF) {
            // если нахожусь в начальном состоянии - возвращаю конец файла
            if (state == 0) return Token(LexemType::eof);
            // если прочитал число - возвращаю число
            if (state == 1) return Token(_value);
            // если прочитал идентификатор - проверяю ключевое слово или обычный идентификатор
            if (state == 5) {
                if (_keywords.count(_lexeme)) {
                    return Token(_keywords[_lexeme]);
                } else {
                    return Token(LexemType::id, _lexeme);
          	}
            }
            // для остальных состояний - ошибка
            return Token(LexemType::error, "неожиданный конец файла");
        }

        char ch = (char)c;

        switch (state) {
            case 0:
                if (isDigit(c)) {
                    state = 1;
                    _value = c - '0';
                } else if (ch == '\'') {
                    state = 2;
                } else if (isLetter(c)) {
                    state = 5;
                    _lexeme += ch;
                } else if (ch == '\"') {
                    state = 4;
                } else if (ch == '!') {
                    state = 7;
                } else if (ch == '<') {
                    state = 8;
                } else if (ch == '=') {
                    state = 9;
                } else if (ch == '+') {
                    state = 10;
                } else if (ch == '|') {
                    state = 11;
                } else if (ch == '&') {
                    state = 12;
                } else if (_punctuation.count(ch)) {
                    // если это знак пунктуации - сразу возвращаю токен
                    return Token(_punctuation[ch]);
                } else if (ch == '>') {
                    return Token(LexemType::opgt);
                } else if (ch == '*') {
                    return Token(LexemType::opmult);
                } else if (ch == '-') {
                    return Token(LexemType::opminus);
                } else if (isWhiteSpace(c)) {
                    continue;
                } else {
                    return Token(LexemType::error, string("неподдерживаемый символ: '") + ch + "'");
                }
                break;

            case 1:
                if (isDigit(c)) {
                    _value = _value * 10 + (c - '0');
                } else {
                    ungetChar();
                    return Token(_value);
                }
                break;

            case 2:
                if (ch == '\'') {
                    return Token(LexemType::error, "пустая символьная константа");
                }
                _value = c;
                state = 3;
                break;

            case 3:
                if (ch == '\'') {
                    return Token((char)_value);
                }
                // любой другой символ - ошибка
                return Token(LexemType::error, "символьная константа содержит более одного символа");

            case 4:
                if (ch == '\"') {
                    return Token(LexemType::str, _lexeme);
                }
                _lexeme += ch;
                break;

            case 5:
                if (isLetter(c) || isDigit(c)) {
                    _lexeme += ch;
                } else {
                    ungetChar();
                    if (_keywords.count(_lexeme)) {
                        return Token(_keywords[_lexeme]);
                    } else {
                        return Token(LexemType::id, _lexeme);
                    }
                }
                break;

            case 7:
                if (ch == '=') {
                    return Token(LexemType::opne);
                }
                ungetChar();
                return Token(LexemType::opnot);

            case 8:
                if (ch == '=') {
                    return Token(LexemType::ople);
                }
                ungetChar();
                return Token(LexemType::oplt);
            case 9:
                if (ch == '=') {
                    return Token(LexemType::opeq);
                }
                ungetChar();
                return Token(LexemType::opassign);

            case 10:
                if (ch == '+') {
                    return Token(LexemType::opinc);
                }
                ungetChar();
                return Token(LexemType::opplus);

            case 11:
                if (ch == '|') {
                    return Token(LexemType::opor);
                }
                return Token(LexemType::error, "одиночный символ '|'");

            case 12:
                if (ch == '&') {
                    return Token(LexemType::opand);
                }
                return Token(LexemType::error, "одиночный символ '&'");
        }
    }
}
