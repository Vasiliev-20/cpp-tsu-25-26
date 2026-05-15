#include "Translator.h"
#include <sstream>
#include <iostream>
#include <memory>

Translator::Translator(std::istream& input)   
    : _scanner(input), _currentLexem(LexemType::eof), _nextLabel(0) {
    nextToken();
}

void Translator::nextToken() {   // Запрашивает у лексера следующий токен и сохраняет в _currentLexem
    _currentLexem = _scanner.getNextToken();
    if (_currentLexem.type() == LexemType::error) {
        lexicalError(_currentLexem.str());
    }
}

void Translator::match(LexemType expected) {  // Проверяет, что текущий токен имеет ожидаемый тип
    if (_currentLexem.type() == expected) {
        nextToken();
    } else {
        std::ostringstream oss;
        oss << "Expected token type " << static_cast<int>(expected)
            << ", got " << static_cast<int>(_currentLexem.type());
        syntaxError(oss.str());
    }
}

void Translator::generateAtom(std::unique_ptr<Atom> atom) {  // Принимает умный указатель unique_ptr<Atom>  - созданный через std::make_unique
    _atoms.push_back(std::move(atom)); //move(atom) — передаёт владение атомом в вектор _atoms
}

void Translator::printAtoms(std::ostream& stream) const { // Выводит все атомы в поток (ADD, '5', 2, 0)
    for (const auto& atom : _atoms) {
        stream << atom->toString() << std::endl;
    }
}

std::shared_ptr<MemoryOperand> Translator::allocTemp() {   // Создаёт временную переменную _tmp1
    return _symTable.alloc();
}

std::shared_ptr<LabelOperand> Translator::newLabel() { // Создаёт новую метку
    return std::make_shared<LabelOperand>(_nextLabel++);
}

void Translator::syntaxError(const std::string& message) {  // Исключение перехватывается в translate() и выводится в cerr
    throw TranslationException("Syntax error: " + message);
}

void Translator::lexicalError(const std::string& message) {
    throw TranslationException("Lexical error: " + message);
}

void Translator::translate() {
    try {
        auto result = E();
        if (!result) syntaxError("Expression parsing failed");
        if (_currentLexem.type() != LexemType::eof)
            syntaxError("Extra tokens after expression");
        printAtoms(std::cout);
        std::cout << "\nSYMBOL TABLE\n" << _symTable;
    } catch (const TranslationException& e) { // catch — блок -  ловит брошенную ошибку и обрабатывает её
        std::cerr << e.what() << std::endl; // what() возвращает строку с сообщением об ошибке
    }
}

// ========== Рекурсивный спуск ==========

std::shared_ptr<RValue> Translator::E() {
    return E7();
}

std::shared_ptr<RValue> Translator::E7() {
    auto q = E6(); // Левая часть
    if (!q) syntaxError("E6 failed in E7");
    auto p = E7_(q); // вызывает E7_ и передаёт ей результат левой части как наследуемый атрибут p
    if (!p) syntaxError("E7_ failed");
    return p;
}

std::shared_ptr<RValue> Translator::E7_(std::shared_ptr<RValue> p) {
    if (_currentLexem.type() == LexemType::opor) {   // если видим ||
        nextToken();                                  // съедаем ||
        auto r = E6();                               // разбираем правый операнд
        if (!r) syntaxError("Expected E6 after ||");
        auto s = allocTemp();                        // создаём временную переменную для результата
        generateAtom(std::make_unique<BinaryOpAtom>("OR", p, r, s));  // генерируем атом OR
        auto q = E7_(s);                             // рекурсивно обрабатываем следующие ||
        if (!q) syntaxError("E7_ after OR failed");
        return q;
    }
    return p;   // если нет || — возвращаем накопленный результат
}

std::shared_ptr<RValue> Translator::E6() {
    auto q = E5();
    if (!q) syntaxError("E5 failed in E6");
    auto p = E6_(q);
    if (!p) syntaxError("E6_ failed");
    return p;
}

std::shared_ptr<RValue> Translator::E6_(std::shared_ptr<RValue> p) {
    if (_currentLexem.type() == LexemType::opand) {
        nextToken();
        auto r = E5();
        if (!r) syntaxError("Expected E5 after &&");
        auto s = allocTemp();
        generateAtom(std::make_unique<BinaryOpAtom>("AND", p, r, s));
        auto q = E6_(s);
        if (!q) syntaxError("E6_ after AND failed");
        return q;
    }
    return p;
}

std::shared_ptr<RValue> Translator::E5() {
    auto q = E4();
    if (!q) syntaxError("E4 failed in E5");
    auto p = E5_(q);
    if (!p) syntaxError("E5_ failed");
    return p;
}

std::shared_ptr<RValue> Translator::E5_(std::shared_ptr<RValue> p) {
    LexemType op = _currentLexem.type();
    if (op == LexemType::opeq || op == LexemType::opne ||
        op == LexemType::opgt || op == LexemType::oplt ||
        op == LexemType::ople) {      
        nextToken();
        auto r = E4();
        if (!r) syntaxError("Expected E4 after relational operator");
        auto s = allocTemp();
        auto l = newLabel();
        generateAtom(std::make_unique<UnaryOpAtom>("MOV", std::make_shared<NumberOperand>(1), s));
        std::string cond;
        switch (op) {
            case LexemType::opeq: cond = "EQ"; break;
            case LexemType::opne: cond = "NE"; break;
            case LexemType::opgt: cond = "GT"; break;
            case LexemType::oplt: cond = "LT"; break;
            case LexemType::ople: cond = "LE"; break;
            default: break;
        }
        generateAtom(std::make_unique<ConditionalJumpAtom>(cond, p, r, l));
        generateAtom(std::make_unique<UnaryOpAtom>("MOV", std::make_shared<NumberOperand>(0), s));
        auto q = E5_(s);
        if (!q) syntaxError("E5_ after comparison failed");
        return q;
    }
    return p;
}

std::shared_ptr<RValue> Translator::E4() {
    auto q = E3();
    if (!q) syntaxError("E3 failed in E4");
    auto p = E4_(q);
    if (!p) syntaxError("E4_ failed");
    return p;
}

std::shared_ptr<RValue> Translator::E4_(std::shared_ptr<RValue> p) {
    LexemType op = _currentLexem.type();
    if (op == LexemType::opplus || op == LexemType::opminus) {
        nextToken();
        auto r = E3();
        if (!r) syntaxError("Expected E3 after +/-");
        auto s = allocTemp();
        std::string atomOp = (op == LexemType::opplus) ? "ADD" : "SUB";
        generateAtom(std::make_unique<BinaryOpAtom>(atomOp, p, r, s));
        auto q = E4_(s);
        if (!q) syntaxError("E4_ after +/- failed");
        return q;
    }
    return p;
}

std::shared_ptr<RValue> Translator::E3() { 
    auto q = E2();
    if (!q) syntaxError("E2 failed in E3");
    auto p = E3_(q);
    if (!p) syntaxError("E3_ failed");
    return p;
}

std::shared_ptr<RValue> Translator::E3_(std::shared_ptr<RValue> p) {  
    if (_currentLexem.type() == LexemType::opmult) {  // * умножение
        nextToken();
        auto r = E2();
        if (!r) syntaxError("Expected E2 after *");
        auto s = allocTemp();
        generateAtom(std::make_unique<BinaryOpAtom>("MUL", p, r, s));
        auto q = E3_(s);
        if (!q) syntaxError("E3_ after * failed");
        return q;
    }
    return p;
}

std::shared_ptr<RValue> Translator::E2() {
    if (_currentLexem.type() == LexemType::opnot) {
        nextToken();
        auto q = E1();
        if (!q) syntaxError("Expected E1 after !");
        auto r = allocTemp();
        generateAtom(std::make_unique<UnaryOpAtom>("NOT", q, r));
        return r;
    }
    return E1();
}

// Реализация E1()
std::shared_ptr<RValue> Translator::E1() {  // num — целое число (например, 3). Создаём NumberOperand(3).
    if (_currentLexem.type() == LexemType::num) {
        int val = _currentLexem.value();
        nextToken();
        return std::make_shared<NumberOperand>(val);
    }
    if (_currentLexem.type() == LexemType::chr) { //  chr — символ в кавычках (например, 'A'). Превращаем ASCII-код в число и создаём NumberOperand(65).
        char ch = static_cast<char>(_currentLexem.value());
        nextToken();
        return std::make_shared<NumberOperand>(ch);
    }
    if (_currentLexem.type() == LexemType::opinc) { //  префиксный инкремент ++id
        nextToken();
        if (_currentLexem.type() != LexemType::id)
            syntaxError("Expected id after ++");
        std::string name = _currentLexem.str();
        nextToken();
        auto q = _symTable.add(name);
        auto r = allocTemp();
        generateAtom(std::make_unique<BinaryOpAtom>("ADD", q, std::make_shared<NumberOperand>(1), r));
        return r;
    }
    if (_currentLexem.type() == LexemType::id) { // идентификатор (переменная) с постфиксным инкрементом
        std::string name = _currentLexem.str();
        nextToken();
        auto p = _symTable.add(name);
        auto q = E1_(p);
        if (!q) syntaxError("E1_ failed");
        return q;
    }
    if (_currentLexem.type() == LexemType::lpar) { // выражение в скобках
        nextToken();
        auto q = E();
        if (!q) syntaxError("Expression expected after '('");
        if (_currentLexem.type() != LexemType::rpar)
            syntaxError("Missing ')'");
        nextToken();
        return q;
    }
    syntaxError("Unexpected token in E1"); //  ошибка
    return nullptr;
}

// Реализация E1_
std::shared_ptr<RValue> Translator::E1_(std::shared_ptr<RValue> p) {
    if (_currentLexem.type() == LexemType::opinc) {
        nextToken();
        auto s = p;                            // s — это операнд переменной
        auto r = allocTemp();                  // временная для старого значения
        generateAtom(std::make_unique<UnaryOpAtom>("MOV", s, r));  // r = s (сохраняем старое)
        generateAtom(std::make_unique<BinaryOpAtom>("ADD", s, std::make_shared<NumberOperand>(1), 
                   std::static_pointer_cast<MemoryOperand>(s)));   // s = s + 1
        return r;                              // возвращаем старое значение
    }
    return p;   // нет ++ — возвращаем сам операнд
}
