#ifndef TRANSLATOR_H
#define TRANSLATOR_H

#include "Scaner.h"
#include "SymbolTable.h"
#include "StringTable.h"
#include "Atoms.h"
#include <memory>
#include <vector>
#include <iostream>

class TranslationException : public std::exception {
    std::string _message;
public:
    TranslationException(const std::string& msg) : _message(msg) {}
    const char* what() const noexcept override { return _message.c_str(); }
};

class Translator {
protected:
    std::vector<std::unique_ptr<Atom>> _atoms;
    SymbolTable _symTable;
    StringTable _strTable;
    Scanner _scanner;
    Token _currentLexem;
    int _nextLabel;

    void nextToken();
    void match(LexemType expected);

public:
    Translator(std::istream& input);
    void generateAtom(std::unique_ptr<Atom> atom);
    void printAtoms(std::ostream& stream) const;
    std::shared_ptr<MemoryOperand> allocTemp();
    std::shared_ptr<LabelOperand> newLabel();
    void syntaxError(const std::string& message);
    void lexicalError(const std::string& message);
    void translate();

    // Методы рекурсивного спуска (грамматика выражений)
    std::shared_ptr<RValue> E();
    std::shared_ptr<RValue> E7();
    std::shared_ptr<RValue> E7_(std::shared_ptr<RValue> p);
    std::shared_ptr<RValue> E6();
    std::shared_ptr<RValue> E6_(std::shared_ptr<RValue> p);
    std::shared_ptr<RValue> E5();
    std::shared_ptr<RValue> E5_(std::shared_ptr<RValue> p);
    std::shared_ptr<RValue> E4();
    std::shared_ptr<RValue> E4_(std::shared_ptr<RValue> p);
    std::shared_ptr<RValue> E3();
    std::shared_ptr<RValue> E3_(std::shared_ptr<RValue> p);
    std::shared_ptr<RValue> E2();
    std::shared_ptr<RValue> E1();
    std::shared_ptr<RValue> E1_(std::shared_ptr<RValue> p);
};

#endif
