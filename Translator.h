#pragma once
#include <exception>
#include <vector>
#include <memory>
#include <iostream>
#include <string>
#include <map>
#include <algorithm>
#include "Atoms.h"
#include "SymbolTable.h"
#include "StringTable.h"
#include "Scaner.h"

class TranslatorException : public std::exception {
    std::string message_;
public:
    TranslatorException(const std::string& message);
    const char* what() const noexcept override;
};

class Translator {
public:
    Translator(std::istream& input) : lexicalAnalyser_(input), 
        currentLexem_(lexicalAnalyser_.getNextLexem()) {}

    void generateAtom(std::unique_ptr<Atom> atom, Scope scope);

    void printAtoms(std::ostream& stream) const;

    std::shared_ptr<LabelOperand> newLabel();

    void syntaxError(const std::string& message);

    void lexicalError(const std::string& message);

    bool translate();

    void nextLexem();

    void saveRegs(std::ostream& stream);

    void loadReg(std::ostream& stream);

    void generateProlog(std::ostream& stream);

    void generateFunction(std::ostream & stream, Scope scope);

    void generateCode(std::ostream& stream);
    //////////////////////////////////

    void StmtList(Scope scope);

    void DeclareStmt(Scope scope);

    void DeclareStmt_(SymbolTable::TableRecord::RecordType p,
        std::string q,
        Scope scope);

    SymbolTable::TableRecord::RecordType Type(Scope scope);

    void DeclVarList_(SymbolTable::TableRecord::RecordType p, Scope scope);

    void InitVar(SymbolTable::TableRecord::RecordType p,
        std::string q,
        Scope scope);

    int ParamList(Scope scope);

    int ParamList_(Scope scope);

    void Stmt(Scope scope);

    void AssignOrCallOp(Scope scope);

    void AssignOrCall(Scope scope);

    void AssignOrCall_(std::string p, Scope scope);

    void WhileOp(Scope scope);

    void ForOp(Scope scope);

    void ForInit(Scope scope);

    std::shared_ptr<RValue> ForExpr(Scope scope);

    void ForLoop(Scope scope);

    void IfOp(Scope scope);

    void ElsePart(Scope scope);

    void SwitchOp(Scope scope);

    void Cases(std::shared_ptr<RValue> p, 
        std::shared_ptr<LabelOperand> end, 
        Scope scope);

    void Cases_(std::shared_ptr<RValue> p, 
        std::shared_ptr<LabelOperand> end,
        std::shared_ptr<LabelOperand>& def, 
        Scope scope);

    void ACase(std::shared_ptr<RValue> p,
        std::shared_ptr<LabelOperand> end,
        std::shared_ptr<LabelOperand>& def,
        Scope scope);

    void IOp(Scope scope);

    void OOp(Scope scope);

    void OOp_(Scope scope);

    int ArgList(Scope scope);

    int ArgList_(Scope scope);

    /////////////////
    std::shared_ptr<RValue> E(Scope scope);

    std::shared_ptr<RValue> E7(Scope scope);

    std::shared_ptr<RValue> E7_(std::shared_ptr<RValue> p, Scope scope);

    std::shared_ptr<RValue> E6(Scope scope);

    std::shared_ptr<RValue> E6_(std::shared_ptr<RValue> p, Scope scope);

    std::shared_ptr<RValue> E5(Scope scope);

    std::shared_ptr<RValue> E5_(std::shared_ptr<RValue> p, Scope scope);

    std::shared_ptr<RValue> E4(Scope scope);

    std::shared_ptr<RValue> E4_(std::shared_ptr<RValue> p, Scope scope);

    std::shared_ptr<RValue> E3(Scope scope);

    std::shared_ptr<RValue> E3_(std::shared_ptr<RValue> p, Scope scope);

    std::shared_ptr<RValue> E2(Scope scope);

    std::shared_ptr<RValue> E1(Scope scope);

    std::shared_ptr<RValue> E1_(const std::string& p, Scope scope);

    const SymbolTable& getSymbolTable() const;

    const StringTable& getStringTable() const;

protected:
    std::map<Scope, std::vector<std::unique_ptr<Atom>>> atoms_;
    SymbolTable symbolTable_;
    StringTable stringTable_;
    Scaner lexicalAnalyser_;
    Token currentLexem_;
    int lastLabelID_ = 0;
    std::vector<Token> lastTokens_;
};

extern std::map<int, std::string> sErrors;
