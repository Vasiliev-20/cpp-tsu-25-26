#ifndef ATOMS_H
#define ATOMS_H

#include <string>
#include <memory>

class StringTable;
class SymbolTable;

// ------------------ Операнды (уже есть) ------------------
class Operand {
public:
    virtual ~Operand() = default;
    virtual std::string toString() const = 0;
};

class RValue : public Operand {
public:
    virtual ~RValue() = default;
};

class NumberOperand : public RValue {
    int _value;
public:
    NumberOperand(int value);
    std::string toString() const override;
};

class MemoryOperand : public RValue {
    int _index;
    const SymbolTable* _symbolTable;
public:
    MemoryOperand(int index, const SymbolTable* symbolTable);
    std::string toString() const override;
    bool operator==(const MemoryOperand& other) const;
};

class StringOperand : public Operand {
    int _index;
    const StringTable* _stringTable;
public:
    StringOperand(int index, const StringTable* stringTable);
    std::string toString() const override;
    bool operator==(const StringOperand& other) const;
};

class LabelOperand : public Operand {
    int _labelID;
public:
    LabelOperand(int labelID);
    std::string toString() const override;
};

// ------------------ Атомы ------------------
class Atom {
public:
    virtual ~Atom() = default;
    virtual std::string toString() const = 0;
};

// Бинарная операция: ADD, SUB, MUL, DIV, AND, OR
class BinaryOpAtom : public Atom {
    std::string _name;
    std::shared_ptr<RValue> _left;
    std::shared_ptr<RValue> _right;
    std::shared_ptr<MemoryOperand> _result;
public:
    BinaryOpAtom(const std::string& name,
                 std::shared_ptr<RValue> left,
                 std::shared_ptr<RValue> right,
                 std::shared_ptr<MemoryOperand> result);
    std::string toString() const override;
};

// Унарная операция: NEG, NOT, MOV
class UnaryOpAtom : public Atom {
    std::string _name;
    std::shared_ptr<RValue> _operand;
    std::shared_ptr<MemoryOperand> _result;
public:
    UnaryOpAtom(const std::string& name,
                std::shared_ptr<RValue> operand,
                std::shared_ptr<MemoryOperand> result);
    std::string toString() const override;
};

// Условный переход: EQ, NE, GT, LT, GE, LE
class ConditionalJumpAtom : public Atom {
    std::string _condition;
    std::shared_ptr<RValue> _left;
    std::shared_ptr<RValue> _right;
    std::shared_ptr<LabelOperand> _label;
public:
    ConditionalJumpAtom(const std::string& condition,
                        std::shared_ptr<RValue> left,
                        std::shared_ptr<RValue> right,
                        std::shared_ptr<LabelOperand> label);
    std::string toString() const override;
};

// Безусловный переход: JMP
class JumpAtom : public Atom {
    std::shared_ptr<LabelOperand> _label;
public:
    JumpAtom(std::shared_ptr<LabelOperand> label);
    std::string toString() const override;
};

// Вывод: OUT
class OutAtom : public Atom {
    std::shared_ptr<Operand> _value;
public:
    OutAtom(std::shared_ptr<Operand> value);
    std::string toString() const override;
};

#endif
