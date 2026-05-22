#pragma once
#include <string>
#include <memory>

class StringTable;
class SymbolTable;

class Operand {
public:
    virtual std::string toString() const = 0;
    virtual ~Operand() = default;
};

class RValue : public Operand {
public:
    virtual void load(std::ostream& stream) const = 0;
};

class MemoryOperand : public RValue {
public:
    MemoryOperand(int index, const SymbolTable* symbolTable);
    std::string toString() const override;
    bool operator==(const MemoryOperand& other);
    int index() const;
    void load(std::ostream& stream) const override;
    void save(std::ostream& stream) const;
    const std::string getName() const;
    const int countArgs() const;
protected:
    int index_;
    const SymbolTable* symbolTable_;
};

class NumberOperand : public RValue {
public:
    explicit NumberOperand(int value);
    std::string toString() const override;
    void load(std::ostream& stream) const override;
protected:
    int value_;
};

class StringOperand : public Operand {
public:
    StringOperand(int index, const StringTable* stringTable);
    int getIndex() { return index_; }
    std::string toString() const override;
    bool operator==(const StringOperand& other);
protected:
    int index_;
    const StringTable* stringTable_;
};

class LabelOperand : public Operand {
public:
    explicit LabelOperand(int labelID);
    std::string toString() const override;
    int getID();
    bool operator==(const LabelOperand& other) const;
    bool operator!=(const LabelOperand& other) const;
protected:
    int labelID_;
};

class Atom {
public:
    virtual std::string toString() const = 0;
    virtual void generate(std::ostream& stream) const = 0;
    virtual ~Atom() = default;
};

class CallAtom : public Atom{
public:
    CallAtom(std::shared_ptr<MemoryOperand> func, std::shared_ptr<MemoryOperand> var);
    std::string toString() const;
    void generate(std::ostream& stream) const override;
protected:
    std::shared_ptr<MemoryOperand> func_;
    std::shared_ptr<MemoryOperand> var_;
};

class RetAtom : public Atom {
public:
    RetAtom(std::shared_ptr<RValue> ret, int retOffset, int varCount);
    std::string toString() const;
    void generate(std::ostream& stream) const override;
private:
    std::shared_ptr<RValue> ret_;
    int retOffset_;
    int localCount_;
};

class ParamAtom : public Atom {
public:
    ParamAtom(std::shared_ptr<RValue> param);
    std::string toString() const;
    void generate(std::ostream& stream) const override;
private:
    std::shared_ptr<RValue> param_;
};

class UnaryOpAtom : public Atom {
public:
    UnaryOpAtom(const std::string& name,
        std::shared_ptr<RValue> operand,
        std::shared_ptr<MemoryOperand> result);
    std::string toString() const;
    void generate(std::ostream& stream) const override;
protected:
    std::string name_;
    std::shared_ptr<RValue> operand_;
    std::shared_ptr<MemoryOperand> result_;
};

class BinaryOpAtom : public Atom {
public:
    BinaryOpAtom(const std::string& name, std::shared_ptr<RValue> left,
        std::shared_ptr<RValue> right, std::shared_ptr<MemoryOperand> result);
    std::string toString() const override;
    void generate(std::ostream& stream) const override;
    void generateOperation(std::ostream& stream) const;
protected:
    std::string name_;
    std::shared_ptr<RValue> left_;
    std::shared_ptr<RValue> right_;
    std::shared_ptr<MemoryOperand> result_;
};

class ConditionalJumpAtom : public Atom {
public:
    ConditionalJumpAtom(const std::string& condition, std::shared_ptr<RValue> left,
        std::shared_ptr<RValue> right, std::shared_ptr<LabelOperand> label);
    std::string toString() const override;
    void generate(std::ostream& stream) const override;
    void generateOperation(std::ostream& stream) const;
protected:
    std::string condition_;
    std::shared_ptr<RValue> left_;
    std::shared_ptr<RValue> right_;
    std::shared_ptr<LabelOperand> label_;
};

class JumpAtom : public Atom {
public:
    explicit JumpAtom(std::shared_ptr<LabelOperand> label);
    std::string toString() const override;
    void generate(std::ostream& stream) const override;
protected:
    std::shared_ptr<LabelOperand> label_;
};

class OutAtom : public Atom {
public:
    explicit OutAtom(std::shared_ptr<Operand> value);
    std::string toString() const override;
    void generate(std::ostream& stream) const override;
protected:
    std::shared_ptr<Operand> value_;
};

class InAtom : public Atom {
public:
    explicit InAtom(std::shared_ptr<MemoryOperand> result);
    std::string toString() const override;
    void generate(std::ostream& stream) const override;
protected:
    std::shared_ptr<MemoryOperand> result_;
};

class LabelAtom : public Atom {
public:
    explicit LabelAtom(std::shared_ptr<LabelOperand> label);
    std::string toString() const override;
    void generate(std::ostream& stream) const override;
protected:
    std::shared_ptr<LabelOperand> label_;
};
