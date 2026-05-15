#ifndef SCANER_H
#define SCANER_H

#include <string>
#include <iostream>
#include <map>

using namespace std;

// тут у меня все типы лексем которые могут быть в нашем языке
enum class LexemType {
    num, chr, str, id, lpar, rpar, lbrace, rbrace, lbracket, rbracket,
    semicolon, comma, colon, opassign, opplus, opminus, opmult, opinc,
    opeq, opne, oplt, opgt, ople, opnot, opor, opand, kwint, kwchar,
    kwif, kwelse, kwswitch, kwcase, kwwhile, kwfor, kwreturn, kwin,
    kwout, eof, error
};

// это класс для токена, тут хранится тип, число если надо и строка если надо
class Token {
    LexemType _type;    // тип лексемы - что это такое
    int _value;         // для чисел и символов - числовое значение
    string _str;        // для идентификаторов, строк и ошибок - текст
public:
    Token(LexemType type) {
        _type = type;
        _value = 0;
        _str = "";
    }
    Token(int value) {
        _type = LexemType::num;
        _value = value;
        _str = "";
    }
    Token(LexemType type, const string &str) {
        _type = type;
        _value = 0;
        _str = str;
    }
    Token(char c) {
        _type = LexemType::chr;
        _value = c;
        _str = "";
    }
    // метод чтобы выводить токен в поток в красивом виде
    void print(ostream &stream);
    // методы доступа
    LexemType type() { return _type; }
    int value() { return _value; }
    string str() { return _str;}
};

// это сканер, он разбирает текст на токены используя конечный автомат
class Scanner {
    istream& _stream;    // откуда читаем символы - файл или строка
    string _lexeme;      // тут накапливаем лексему когда читаем идентификатор или строку
    int _value;          // для чисел - тут храним числовое значение
    // таблицы для быстрого поиска чтобы не писать кучу if-ов
    map<char, LexemType> _punctuation;  // знаки пунктуации типа скобок и запятых
    map<string, LexemType> _keywords;   // ключевые слова типа int, return и тд
    // вспомогательные методы которые используются внутри
    int getChar();        // получить символ из потока
    void ungetChar();     // вернуть символ обратно в поток если прочитали лишнее
    bool isWhiteSpace(int c);  // проверка на пробельный символ - пробел, табуляция, перенос
    bool isDigit(int c);       // проверка на цифру от 0 до 9
    bool isLetter(int c);      // проверка на букву или подчеркивание
public:
    Scanner(istream& stream);  // конструктор - принимает поток откуда читать
    Token getNextToken();      // главный метод - получить следующий токен из потока
};

#endif
