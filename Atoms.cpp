#include "Atoms.h"
#include "StringTable.h"
#include "SymbolTable.h"

MemoryOperand::MemoryOperand(int index, const SymbolTable* st) : index_(index), symbolTable_(st) {}

std::string MemoryOperand::toString() const {
    return std::to_string(index_);
}

bool MemoryOperand::operator==(const MemoryOperand& other) {
    return index_ == other.index_ && symbolTable_ == other.symbolTable_;
}

int MemoryOperand::index() const {
    return index_;
}

void MemoryOperand::load(std::ostream& stream) const {
    const auto& record = (*symbolTable_)[index_];
    if (record.offset_ == -1) {
        stream << "LDA " << record.name() << "\n";
    }
    else {
        stream << "LXI H, " << record.offset_ << "\n";
        stream << "DAD SP\n";
        stream << "MOV A, M\n";
    }
}

const std::string MemoryOperand::getName() const {
    return (*symbolTable_)[index_].name_;
}

const int MemoryOperand::countArgs() const {
    return (*symbolTable_)[index_].len_;
}

void MemoryOperand::save(std::ostream& stream) const {
    const auto& record = (*symbolTable_)[index_];
    if (record.offset_ == -1) {
        stream << "STA " << record.name() << "\n";
    }
    else {
        stream << "LXI H, " << record.offset_ << "\n";
        stream << "DAD SP\n";
        stream << "MOV M, A\n";
    }
}

NumberOperand::NumberOperand(int value) : value_(value) {}

std::string NumberOperand::toString() const {
    return "'" + std::to_string(value_) + "'";
}

StringOperand::StringOperand(int index, const StringTable* stringTable)
    : index_(index), stringTable_(stringTable) {}

std::string StringOperand::toString() const {
    return "S" + std::to_string(index_);
}

void NumberOperand::load(std::ostream& stream) const {
    stream << "MVI A, " << value_ << "\n";
}

bool StringOperand::operator==(const StringOperand& other) {
    return index_ == other.index_ && stringTable_ == other.stringTable_;
}

LabelOperand::LabelOperand(int labelID) : labelID_(labelID) {}

std::string LabelOperand::toString() const {
    return "L" + std::to_string(labelID_);
}

int LabelOperand::getID() {
    return labelID_;
}

bool LabelOperand::operator==(const LabelOperand& other) const {
    return labelID_ == other.labelID_;
}

bool LabelOperand::operator!=(const LabelOperand& other) const {
    return labelID_ != other.labelID_;
}

CallAtom::CallAtom(std::shared_ptr<MemoryOperand> func, std::shared_ptr<MemoryOperand> var) :
    func_(func), var_(var) {
}

std::string CallAtom::toString() const {
    return "(CALL, " + func_->toString() + ", , " + var_->toString() + ")";
}

void CallAtom::generate(std::ostream& stream) const {
    stream << "; " << toString() << "\n";
    stream << "CALL " << func_->getName() << "\n";
    for (int i = 0; i < func_->countArgs(); i++) {
        stream << "POP B\n";
    }
    var_->save(stream);
}

RetAtom::RetAtom(std::shared_ptr<RValue> ret, int retOffset, int varCount)
    : ret_(ret), retOffset_(retOffset), localCount_(varCount) {}

std::string RetAtom::toString() const {
    return "(RET, , , " + ret_->toString() + ")";
}

void RetAtom::generate(std::ostream& stream) const {
    stream << "; " << toString() << "\n";
    ret_->load(stream);
    stream << "LXI H, " << retOffset_ << "\n";
    stream << "DAD SP\n";
    stream << "MOV M, A\n";
    for (int i = 0; i < localCount_; ++i) {
        stream << "POP B\n";
    }
    stream << "RET\n";
}

ParamAtom::ParamAtom(std::shared_ptr<RValue> param) : param_(param) {}

std::string ParamAtom::toString() const {
    return "(PARAM, , , " + param_->toString() + ")";
}

void ParamAtom::generate(std::ostream& stream) const {
    stream << "; " << toString() << "\n";
    param_->load(stream);
    stream << "MOV C, A\n";
    stream << "MVI B, 0\n";
    stream << "PUSH B\n";
}

UnaryOpAtom::UnaryOpAtom(const std::string& name, std::shared_ptr<RValue> op, std::shared_ptr<MemoryOperand> res)
    : name_(name), operand_(op), result_(res) {}

std::string UnaryOpAtom::toString() const {
    return "(" + name_ + ", " + operand_->toString() + ", , " + result_->toString() + ")";
}

void UnaryOpAtom::generate(std::ostream& stream) const {
    stream << "; " << this->toString() << "\n";
    operand_->load(stream);
    if (name_ == "MOV") {}
    else if (name_ == "NOT") {
        stream << "CMA\n";
    }
    result_->save(stream);
}

BinaryOpAtom::BinaryOpAtom(const std::string& name, std::shared_ptr<RValue> left,
    std::shared_ptr<RValue> right, std::shared_ptr<MemoryOperand> result)
    : name_(name), left_(left), right_(right), result_(result) {}

std::string BinaryOpAtom::toString() const {
    return "(" + name_ + ", " + left_->toString() + ", " +
        right_->toString() + ", " + result_->toString() + ")";
}

void BinaryOpAtom::generate(std::ostream& stream) const {
    stream << "; " << this->toString() << std::endl;
    right_->load(stream);
    generateOperation(stream);
    result_->save(stream);
}

void BinaryOpAtom::generateOperation(std::ostream& stream) const {
    if (name_ == "MUL" || name_ == "DIV") {
        stream << "MOV D, A" << "\n";
        left_->load(stream);
        if (name_ == "MUL") {
            stream << "CALL @MULT\n";
            stream << "MOV A, C\n";
        }
        else if (name_ == "DIV") {
            stream << "CALL @DIV\n";
            stream << "MOV A, C\n";
        }
    }
    else { // ADD, SUB, AND, OR
        stream << "MOV B, A" << "\n";
        left_->load(stream);
        if (name_ == "ADD") {
            stream << "ADD B\n";
        }
        else if (name_ == "SUB") {
            stream << "SUB B\n";
        }
        else if (name_ == "AND") {
            stream << "ANA B\n";
        }
        else if (name_ == "OR") {
            stream << "ORA B\n";
        }
    }
}

JumpAtom::JumpAtom(std::shared_ptr<LabelOperand> label) : label_(label) {}

std::string JumpAtom::toString() const {
    return "(JMP, , , " + label_->toString() + ")";
}

void JumpAtom::generate(std::ostream& stream) const {
    stream << "; " << this->toString() <<"\n";
    stream << "JMP LBL" << label_->getID() << "\n";
}

ConditionalJumpAtom::ConditionalJumpAtom(const std::string& condition, std::shared_ptr<RValue> left,
    std::shared_ptr<RValue> right, std::shared_ptr<LabelOperand> label)
    : condition_(condition), left_(left), right_(right), label_(label) {
}

std::string ConditionalJumpAtom::toString() const {
    return "(" + condition_ + ", " + left_->toString() + ", " +
        right_->toString() + ", " + label_->toString() + ")";
}

void ConditionalJumpAtom::generate(std::ostream& stream) const {
    stream << "; " << this->toString() << "\n";
    left_->load(stream);
    stream << "MOV B, A\n";
    right_->load(stream);
    generateOperation(stream);
}

void ConditionalJumpAtom::generateOperation(std::ostream& stream) const {
    if (condition_ == "LE") {
        stream << "JZ LBL" << label_->getID() << "\n";
        stream << "JM LBL" << label_->getID() << "\n";
    }
    else { // EQ, NE, LT, GT
        stream << "CMP B\n";
        if (condition_ == "EQ") {
            stream << "JZ LBL" << label_->getID() << "\n";
        }
        else if (condition_ == "NE") {
            stream << "JNZ LBL" << label_->getID() << "\n";
        }
        else if (condition_ == "LT") {
            stream << "JM LBL" << label_->getID() << "\n";
        }
        else if (condition_ == "GT") {
            std::string skipLabel = "LBL" + std::to_string(label_->getID()) + "_skip";
            stream << "JZ " << skipLabel << "\n";
            stream << "JP LBL" << label_->getID() << "\n";
            stream << skipLabel << ":\n";
        }
    }
}

OutAtom::OutAtom(std::shared_ptr<Operand> value) : value_(value) {}

std::string OutAtom::toString() const {
    return "(OUT, , , " + value_->toString() + ")";
}

void OutAtom::generate(std::ostream& stream) const {
    stream << "; " << this->toString() << "\n";
    if (auto rOperand = std::dynamic_pointer_cast<RValue>(value_)) {
        rOperand->load(stream);
    }
    else if (auto sOperand = std::dynamic_pointer_cast<StringOperand>(value_)) {
        stream << "LXI A, str" << sOperand->getIndex() << "\n";
    }
    stream << "CALL @PRINT\n";
}

InAtom::InAtom(std::shared_ptr<MemoryOperand> result) : result_(result) {}

std::string InAtom::toString() const {
    return "(IN, , , " + result_->toString() + ")";
}

void InAtom::generate(std::ostream& stream) const {
    stream << "IN 0" << "\n";
    result_->save(stream);
}

LabelAtom::LabelAtom(std::shared_ptr<LabelOperand> label) : label_(label) {}

std::string LabelAtom::toString() const {
    return "(LBL, , , " + label_->toString() + ")";
}

void LabelAtom::generate(std::ostream& stream) const {
    stream << "LBL" << label_->getID() << ":\n";
}
