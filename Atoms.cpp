#include "Atoms.h"
#include "StringTable.h"
#include "SymbolTable.h"
#include <sstream>

// ------------------ Операнды (реализация) ------------------
NumberOperand::NumberOperand(int value) : _value(value) {}  // Конструктор просто сохраняет число.
std::string NumberOperand::toString() const {
    return "'" + std::to_string(_value) + "'";
}

MemoryOperand::MemoryOperand(int index, const SymbolTable* symbolTable) // Конструктор cохраняет индекс переменной и указатель на таблицу символов.
    : _index(index), _symbolTable(symbolTable) {}
std::string MemoryOperand::toString() const {   //  возвращает индекс как строку, без кавычек
    return std::to_string(_index);
}
bool MemoryOperand::operator==(const MemoryOperand& other) const {
    return _index == other._index && _symbolTable == other._symbolTable;
}

StringOperand::StringOperand(int index, const StringTable* stringTable)
    : _index(index), _stringTable(stringTable) {}
std::string StringOperand::toString() const {
    return std::to_string(_index);
}
bool StringOperand::operator==(const StringOperand& other) const {
    return _index == other._index && _stringTable == other._stringTable;
}

LabelOperand::LabelOperand(int labelID) : _labelID(labelID) {} //  хранит номер метки, возвращает его строкой.
std::string LabelOperand::toString() const {
    return std::to_string(_labelID);
}

// ------------------ Атомы (реализация) ------------------
BinaryOpAtom::BinaryOpAtom(const std::string& name,
                           std::shared_ptr<RValue> left,
                           std::shared_ptr<RValue> right,
                           std::shared_ptr<MemoryOperand> result)
    : _name(name), _left(left), _right(right), _result(result) {}

std::string BinaryOpAtom::toString() const {
    std::ostringstream oss;
    oss << "(" << _name
        << ", " << _left->toString()
        << ", " << _right->toString()
        << ", " << _result->toString() << ")";
    return oss.str();
}

UnaryOpAtom::UnaryOpAtom(const std::string& name,
                         std::shared_ptr<RValue> operand,
                         std::shared_ptr<MemoryOperand> result)
    : _name(name), _operand(operand), _result(result) {}

std::string UnaryOpAtom::toString() const {
    std::ostringstream oss;
    oss << "(" << _name
        << ", " << _operand->toString()
        << ", " << _result->toString() << ")";
    return oss.str();
}

ConditionalJumpAtom::ConditionalJumpAtom(const std::string& condition,
                                         std::shared_ptr<RValue> left,
                                         std::shared_ptr<RValue> right,
                                         std::shared_ptr<LabelOperand> label)  // Если условие истинно, управление передаётся на метку _label.
    : _condition(condition), _left(left), _right(right), _label(label) {}

std::string ConditionalJumpAtom::toString() const {
    std::ostringstream oss;
    oss << "(" << _condition
        << ", " << _left->toString()
        << ", " << _right->toString()
        << ", " << _label->toString() << ")";
    return oss.str();
}

JumpAtom::JumpAtom(std::shared_ptr<LabelOperand> label) : _label(label) {}  // Безусловный переход — всегда идёт на метку.

std::string JumpAtom::toString() const {
    std::ostringstream oss;
    oss << "(JMP, " << _label->toString() << ")";
    return oss.str();
}

OutAtom::OutAtom(std::shared_ptr<Operand> value) : _value(value) {} // Вывод значения

std::string OutAtom::toString() const {
    std::ostringstream oss;
    oss << "(OUT, " << _value->toString() << ")";
    return oss.str();
}
