#include "Translator.h"

TranslatorException::TranslatorException(const std::string& message) : message_(message) {}

const char* TranslatorException::what() const noexcept {
    return message_.c_str();
}

void Translator::generateAtom(std::unique_ptr<Atom> atom, Scope scope) {
    atoms_[scope].push_back(std::move(atom));
}

void Translator::printAtoms(std::ostream& os) const {
    for (const auto& [scope, atomList] : atoms_) {
        for (const auto& atom : atomList) {
            os << std::left << std::setw(8) << scope << std::setw(8) << atom->toString() << std::endl;
        }
    }
    os << std::endl;
}

std::shared_ptr<LabelOperand> Translator::newLabel() {
    return std::make_shared<LabelOperand>(lastLabelID_++);
}

void Translator::syntaxError(const std::string& message) {
    std::string errorMessage = "Syntax Error: " + message + "\n";

    errorMessage += "last tokens: ";
    for (auto& token : lastTokens_) {
        errorMessage += token.toStringLexem(token.type()) + " ";
    }

    errorMessage += "\ncurrent token: " + currentLexem_.toStringLexem(currentLexem_.type());

    throw TranslatorException(errorMessage);
}

void Translator::lexicalError(const std::string& message) {
    std::string errorMessage = "Lexical Error: ";
    errorMessage += "\nafter: ";
    for (auto& token : lastTokens_) {
        errorMessage += token.toStringLexem(token.type()) + " ";
    }
    errorMessage += "\nerror: ";
    errorMessage += message;
    throw TranslatorException(errorMessage);
}

void Translator::nextLexem() {
    if (currentLexem_.type() != LexemType::error) {
        lastTokens_.push_back(currentLexem_);
        if (lastTokens_.size() > 3) {
            lastTokens_.erase(lastTokens_.begin());
        }
    }

    currentLexem_ = lexicalAnalyser_.getNextLexem();
    if (currentLexem_.type() == LexemType::error) {
        lexicalError(errors[currentLexem_.value()]);
    }
}

void Translator::saveRegs(std::ostream& stream) {
    // Сохраняем регистровые пары в стек
    stream << "PUSH B\n"; // Сохраняет пару BC
    stream << "PUSH D\n"; // Сохраняет пару DE
    stream << "PUSH H\n"; // Сохраняет пару HL (указатель данных)
    stream << "PUSH PSW\n";
}

void Translator::loadReg(std::ostream& stream) {
    // Восстанавливаем регистровые пары из стека в обратном порядке
    stream << "POP PSW\n";
    stream << "POP H\n";  // Восстанавливает HL
    stream << "POP D\n";  // Восстанавливает DE
    stream << "POP B\n";  // Восстанавливает BC
}

void Translator::generateProlog(std::ostream& stream) {
    stream << "ORG 0\n"
        << "LXI H, 0\n"
        << "SPHL\n"
        << "CALL main\n"
        << "END\n"
        << "@MULT:\n"
        << "; Code for MULT library function\n"
        << "@PRINT:\n"
        << "; Code for PRINT library function\n";
}

void Translator::generateFunction(std::ostream& stream, Scope scope) {
    const auto& function = symbolTable_[scope];
    stream << function.name() << ":\n";
    int countLocals = symbolTable_.getM(scope);
    if (countLocals > 0) {
        stream << "LXI B, 0\n";
        for (int i = 0; i < countLocals; ++i) {
            stream << "PUSH B\n";
        }
    }
    for (const auto& atom : atoms_[scope]) {
        atom->generate(stream);
    }
}

void Translator::generateCode(std::ostream& stream) {
    symbolTable_.calculateOffset();

    getSymbolTable().print(std::cout);
    std::cout << std::endl;
    std::cout << getStringTable();
    std::cout << std::endl;

    stream << "ORG 8000H\n";

    symbolTable_.generateGlobals(stream);

    stringTable_.generateStrings(stream);

    generateProlog(stream);

    for (size_t i = 0; i < symbolTable_.size(); ++i) {
        if (symbolTable_[i].kind_ == SymbolTable::TableRecord::RecordKind::func) {
            generateFunction(stream, i);
        }
    }
}

bool Translator::translate() {
    StmtList(GlobalScope);
    if (currentLexem_.type() != LexemType::eof) {
        syntaxError(sErrors[3] + " in S -> StmtList EOF");
    }
    auto mainFunc = symbolTable_.checkFunc("main", 0);
    if (!mainFunc) {
        syntaxError("function 'main' with 0 parameters not found");
    }
    return true;
}

void Translator::StmtList(Scope scope) {
    while (
        currentLexem_.type() == LexemType::kwint ||
        currentLexem_.type() == LexemType::kwchar ||
        currentLexem_.type() == LexemType::id ||
        currentLexem_.type() == LexemType::kwwhile ||
        currentLexem_.type() == LexemType::kwfor ||
        currentLexem_.type() == LexemType::kwif ||
        currentLexem_.type() == LexemType::kwswitch ||
        currentLexem_.type() == LexemType::kwin ||
        currentLexem_.type() == LexemType::kwout ||
        currentLexem_.type() == LexemType::lbrace ||
        currentLexem_.type() == LexemType::kwreturn ||
        currentLexem_.type() == LexemType::semicolon
        ) {
        Stmt(scope);
    }
}

void Translator::DeclareStmt(Scope scope) {
    auto q = Type(scope);
    if (q == SymbolTable::TableRecord::RecordType::unknown) {
        syntaxError(sErrors[1] + " in DeclareStmt -> Type id DeclareStmt_. Type is unknown");
    }
    if (currentLexem_.type() == LexemType::eof) {
        syntaxError(sErrors[2] + " in DeclareStmt -> Type id DeclareStmt_. Expected id");
    }
    if (currentLexem_.type() != LexemType::id) {
        syntaxError(sErrors[1] + " in DeclareStmt -> Type id DeclareStmt_. Expected id");
    }
    std::string name = currentLexem_.str();
    nextLexem();
    DeclareStmt_(q, name, scope);
}

void Translator::DeclareStmt_(SymbolTable::TableRecord::RecordType p,
    std::string q,
    Scope scope) {
    if (currentLexem_.type() == LexemType::lpar) {
        nextLexem();
        if (scope != GlobalScope) {
            syntaxError(sErrors[1] + " in DeclareStmt_ -> ( ParamList ) { StmtList }. " 
                + "Function must be in global scope");
        }
        auto funcOp = symbolTable_.addFunc(q, p, 0);
        if (!funcOp) { 
            syntaxError(sErrors[1] + " in DeclareStmt_ -> ( ParamList ) { StmtList }. " 
                + "Function already defined");
        }
        Scope newScope = funcOp->index();
        int paramNum = ParamList(newScope);
        symbolTable_.updateFuncLen(newScope, paramNum);
        if (currentLexem_.type() == LexemType::eof) {
            syntaxError(sErrors[2] + " in DeclareStmt_ -> ( ParamList ) { StmtList }. Expected ')'");
        }
        if (currentLexem_.type() != LexemType::rpar) {
            syntaxError(sErrors[1] + " in DeclareStmt_ -> ( ParamList ) { StmtList }. Expected ')'");
        }
        nextLexem();
        if (currentLexem_.type() == LexemType::eof) {
            syntaxError(sErrors[2] + " in DeclareStmt_ -> ( ParamList ) { StmtList }. Expected '{'");
        }
        if (currentLexem_.type() != LexemType::lbrace) {
            syntaxError(sErrors[1] + " in DeclareStmt_ -> ( ParamList ) { StmtList }. Expected '{'");
        }
        nextLexem();
        StmtList(newScope);
        if (currentLexem_.type() == LexemType::eof) {
            syntaxError(sErrors[2] + " in DeclareStmt_ -> ( ParamList ) { StmtList }. Expected '}'");
        }
        if (currentLexem_.type() != LexemType::rbrace) {
            syntaxError(sErrors[1] + " in DeclareStmt_ -> ( ParamList ) { StmtList }. Expected '}'");
        }
        nextLexem();
        auto zero = std::make_shared<NumberOperand>(0);
        int retOffset = symbolTable_.getReturnOffset(newScope);
        int varCount = symbolTable_.getM(newScope);
        generateAtom(std::make_unique<RetAtom>(zero, retOffset, varCount), newScope);
        return;
    }
    if (currentLexem_.type() == LexemType::opassign) {
        nextLexem();
        if (currentLexem_.type() == LexemType::eof) {
            syntaxError(sErrors[2] + " in DeclareStmt_ -> = num. Expected number or char");
        }
        if (currentLexem_.type() != LexemType::num &&
            currentLexem_.type() != LexemType::chr) {
            syntaxError(sErrors[1] + " in DeclareStmt_ -> = num DeclVarList_. Expected number or char");
        }
        int val = currentLexem_.value();
        auto var = symbolTable_.addVar(q, scope, p, val);
        if (!var) {
            syntaxError(sErrors[1] + " in DeclareStmt_ -> = num DeclVarList_. Duplicate variable");
        }
        nextLexem();
        DeclVarList_(p, scope);
        if (currentLexem_.type() == LexemType::eof) {
            syntaxError(sErrors[2] + " in DeclareStmt_ -> = num DeclVarList_. Expected ';'");
        }
        if (currentLexem_.type() != LexemType::semicolon) {
            syntaxError(sErrors[1] + " in DeclareStmt_ -> = num DeclVarList_. Expected ';'");
        }
        nextLexem();
        return;
    }
    auto var = symbolTable_.addVar(q, scope, p);
    if (!var) {
        syntaxError(sErrors[1] + " in DeclareStmt_ -> DeclVarList_. Duplicate variable");
    }
    DeclVarList_(p, scope);
    if (currentLexem_.type() == LexemType::eof) {
        syntaxError(sErrors[2] + " in DeclareStmt_ -> DeclVarList_. Expected ';'");
    }
    if (currentLexem_.type() != LexemType::semicolon) {
        syntaxError(sErrors[1] + " in DeclareStmt_ -> DeclVarList_. Expected ';'");
    }
    nextLexem();
}

SymbolTable::TableRecord::RecordType Translator::Type(Scope scope) {
    if (currentLexem_.type() == LexemType::kwint) {
        nextLexem();
        return SymbolTable::TableRecord::RecordType::integer;
    }
    if (currentLexem_.type() == LexemType::kwchar) {
        nextLexem();
        return SymbolTable::TableRecord::RecordType::chr;
    }
    return SymbolTable::TableRecord::RecordType::unknown;
}

void Translator::DeclVarList_(SymbolTable::TableRecord::RecordType p, Scope scope) {
    if (currentLexem_.type() == LexemType::comma) {
        nextLexem();
        if (currentLexem_.type() == LexemType::eof) {
            syntaxError(sErrors[2] + " in DeclVarList_ -> , id InitVar DeclVarList_. Expected id");
        }
        if (currentLexem_.type() != LexemType::id) {
            syntaxError(sErrors[1] + " in DeclVarList_ -> , id InitVar DeclVarList_. Expected id");
        }
        std::string name = currentLexem_.str();
        nextLexem();
        InitVar(p, name, scope);
        DeclVarList_(p, scope);
    }
}

void Translator::InitVar(SymbolTable::TableRecord::RecordType p,
    std::string q,
    Scope scope) {
    if (currentLexem_.type() == LexemType::opassign) {
        nextLexem();
        if (currentLexem_.type() == LexemType::eof){
            syntaxError(sErrors[2] + " in InitVar -> = num. Expected number or char");
        }
        if (currentLexem_.type() != LexemType::num &&
            currentLexem_.type() != LexemType::chr) {
            syntaxError(sErrors[1] + " in InitVar -> = num. Expected number or char");
        }
        int val = currentLexem_.value();
        auto var = symbolTable_.addVar(q, scope, p, val);
        if (!var) {
            syntaxError(sErrors[2] + " in InitVar -> = num. Duplicate variable");
        }
        nextLexem();
        return;
    }
    auto var = symbolTable_.addVar(q, scope, p);
    if (!var) {
        syntaxError(sErrors[2] + " in InitVar -> epsilon. Duplicate variable");
    }
}

int Translator::ParamList(Scope scope) {
    if (currentLexem_.type() == LexemType::rpar) {
        return 0;
    }
    auto type = Type(scope);
    if (type == SymbolTable::TableRecord::RecordType::unknown) {
        syntaxError(sErrors[1] + "Paramlist -> , Type id ParamList_. Expected parameter type after ','");
    }
    if (currentLexem_.type() == LexemType::eof) {
        syntaxError(sErrors[2] + "Paramlist -> , Type id ParamList_. Expected parameter name");
    }
    if (currentLexem_.type() != LexemType::id) {
        syntaxError(sErrors[1] + "Paramlist -> , Type id ParamList_. Expected parameter name");
    }
    std::string name = currentLexem_.str();
    auto var = symbolTable_.addVar(name, scope, type);
    if (!var) {
        syntaxError(sErrors[1] + "Paramlist -> , Type id ParamList_. Duplicate parameter name");
    }
    nextLexem();
    return ParamList_(scope) + 1;
}

int Translator::ParamList_(Scope scope) {
    if (currentLexem_.type() == LexemType::comma) {
        nextLexem();
        auto type = Type(scope);
        if (type == SymbolTable::TableRecord::RecordType::unknown) {
            syntaxError(sErrors[1] + "Paramlist_ -> , Type id ParamList_. Expected parameter type after ','");
        }
        if (currentLexem_.type() == LexemType::eof) {
            syntaxError(sErrors[2] + "Paramlist_ -> , Type id ParamList_. Expected parameter name");
        }
        if (currentLexem_.type() != LexemType::id) {
            syntaxError(sErrors[1] + "Paramlist_ -> , Type id ParamList_. Expected parameter name");
        }
        std::string name = currentLexem_.str();
        auto var = symbolTable_.addVar(name, scope, type);
        if (!var) {
            syntaxError(sErrors[1] + "Paramlist_ -> , Type id ParamList_. Duplicate parameter name");
        }
        nextLexem();
        return ParamList_(scope) + 1;
    }
    return 0;
}

void Translator::Stmt(Scope scope) {
    if (currentLexem_.type() == LexemType::kwint ||
        currentLexem_.type() == LexemType::kwchar) {
        DeclareStmt(scope);
        return;
    }
    if (currentLexem_.type() == LexemType::id) {
        if (scope == GlobalScope) {
            syntaxError(sErrors[1] + " in Stmt -> AssignOrCallOp. " +
                "Value assignment or calling function must be inside function");
        }
        AssignOrCallOp(scope);
        return;
    }
    if (currentLexem_.type() == LexemType::kwwhile) {
        if (scope == GlobalScope) {
            syntaxError(sErrors[1] + " in Stmt -> WhileOp. 'while' must be inside function");
        }
        WhileOp(scope);
        return;
    }
    if (currentLexem_.type() == LexemType::kwfor) {
        if (scope == GlobalScope) {
            syntaxError(sErrors[1] + " in Stmt -> ForOp. 'for' must be inside function");
        }
        ForOp(scope);
        return;
    }
    if (currentLexem_.type() == LexemType::kwif) {
        if (scope == GlobalScope) {
            syntaxError(sErrors[1] + " in Stmt -> IfOp. 'if' must be inside function");
        }
        IfOp(scope);
        return;
    }
    if (currentLexem_.type() == LexemType::kwswitch) {
        if (scope == GlobalScope) {
            syntaxError(sErrors[1] + " in Stmt -> SwitchOp. 'switch' must be inside function");
        }
        SwitchOp(scope);
        return;
    }
    if (currentLexem_.type() == LexemType::kwin) {
        if (scope == GlobalScope) {
            syntaxError(sErrors[1] + " in Stmt -> IOp. 'in' must be inside function");
        }
        IOp(scope);
        return;
    }
    if (currentLexem_.type() == LexemType::kwout) {
        if (scope == GlobalScope) {
            syntaxError(sErrors[1] + " in Stmt -> OOp. 'out' must be inside function");
        }
        OOp(scope);
        return;
    }
    if (currentLexem_.type() == LexemType::lbrace) {
        if (scope == GlobalScope) {
            syntaxError(sErrors[1] + " in Stmt -> { StmtList }. Block must be inside function");
        }
        nextLexem();
        StmtList(scope);
        if (currentLexem_.type() == LexemType::eof) {
            syntaxError(sErrors[2] + " in Stmt -> { StmtList }. Expected '}'");
        }
        if (currentLexem_.type() != LexemType::rbrace) {
            syntaxError(sErrors[1] + " in Stmt -> { StmtList }. Expected '}'");
        }
        nextLexem();
        return;
    }
    if (currentLexem_.type() == LexemType::kwreturn) {
        if (scope == GlobalScope) {
            syntaxError(sErrors[1] + " in Stmt -> return E. 'return' must be inside function");
        }
        nextLexem();
        auto p = E(scope);
        if (!p) {
            syntaxError("");
        }
        if (currentLexem_.type() == LexemType::eof) {
            syntaxError(sErrors[2] + " in Stmt -> return E. Expected ';' after return");
        }
        if (currentLexem_.type() != LexemType::semicolon) {
            syntaxError(sErrors[1] + " in Stmt -> return E. Expected ';' after return");
        }
        int retOffset = symbolTable_.getReturnOffset(scope);
        int varCount = symbolTable_.getM(scope);
        generateAtom(std::make_unique<RetAtom>(p, retOffset, varCount), scope);
        nextLexem();
        return;
    }
    if (currentLexem_.type() == LexemType::semicolon) {
        nextLexem();
        return;
    }
    if (currentLexem_.type() == LexemType::eof) {
        syntaxError(sErrors[1] + " in Stmt. Invalid statement");
    }
    syntaxError(sErrors[1] + " in Stmt. Invalid statement");
}

void Translator::AssignOrCallOp(Scope scope) {
    AssignOrCall(scope);
    if (currentLexem_.type() == LexemType::eof) {
        syntaxError(sErrors[2] + " in AssignOrCallOp -> AssignOrCall ;. Expected ';'");
    }
    if (currentLexem_.type() != LexemType::semicolon) {
        syntaxError(sErrors[1] + " in AssignOrCallOp -> AssignOrCall ;. Expected ';'");
    }
    nextLexem();
}

void Translator::AssignOrCall(Scope scope) {
    if (currentLexem_.type() == LexemType::eof) {
        syntaxError(sErrors[2] + " in AssignOrCall -> id AssignOrCall_. Expected id");
    }
    if (currentLexem_.type() != LexemType::id) {
        syntaxError(sErrors[1] + " in AssignOrCall -> id AssignOrCall_. Expected id");
    }
    std::string name = currentLexem_.str();
    nextLexem();
    AssignOrCall_(name, scope);
}

void Translator::AssignOrCall_(std::string p, Scope scope) {
    if (currentLexem_.type() == LexemType::opassign) {
        nextLexem();
        auto q = E(scope);
        if (!q) {
            syntaxError(sErrors[1] + " in AssignOrCall_ -> = E. Expected expression");
        }
        auto r = symbolTable_.checkVar(scope, p);
        if (!r) {
            syntaxError(sErrors[1] + " in AssignOrCall_ -> = E. Unknown id: " + p);
        }
        generateAtom(std::make_unique<UnaryOpAtom>("MOV", q, r), scope);
        return;
    }
    if (currentLexem_.type() == LexemType::lpar) {
        nextLexem();
        int n = ArgList(scope);
        if (currentLexem_.type() == LexemType::eof) {
            syntaxError(sErrors[2] + " in AssignOrCall_ -> (ArgList). Expected ')'");
        }
        if (currentLexem_.type() != LexemType::rpar) {
            syntaxError(sErrors[1] + " in AssignOrCall_ -> (ArgList). Expected ')'");
        }
        nextLexem();
        auto q = symbolTable_.checkFunc(p, n);
        if (!q) {
            syntaxError(sErrors[1] + " in AssignOrCall_ -> (ArgList). Unknown id: " + p);
        }
        auto r = symbolTable_.alloc(scope);
        generateAtom(std::make_unique<CallAtom>(q, r), scope);
        return;
    }
    if (currentLexem_.type() == LexemType::eof) {
        syntaxError(sErrors[2] + " in AssignOrCall_. Expected value assignment or calling function");
    }
    syntaxError(sErrors[1] + " in AssignOrCall_. Expected value assignment or calling function");
}

void Translator::WhileOp(Scope scope) {
    if (currentLexem_.type() == LexemType::kwwhile) {
        if (scope == GlobalScope) {
            syntaxError(sErrors[1] + " in WhileOp -> while (E) Stmt. 'while' must be inside function");
        }
        nextLexem();
        auto l1 = newLabel();
        generateAtom(std::make_unique<LabelAtom>(l1), scope);
        if (currentLexem_.type() == LexemType::eof) {
            syntaxError(sErrors[2] + " in WhileOp -> while (E) Stmt. Expexted '('");
        }
        if (currentLexem_.type() != LexemType::lpar) {
            syntaxError(sErrors[1] + " in WhileOp -> while (E) Stmt. Expexted '('");
        }
        nextLexem();
        auto p = E(scope);
        if (!p) {
            syntaxError(sErrors[1] + " in WhileOp -> while (E) Stmt. Expected expression");
        }
        if (currentLexem_.type() == LexemType::eof) {
            syntaxError(sErrors[2] + " in WhileOp -> while (E) Stmt. Expected ')'");
        }
        if (currentLexem_.type() != LexemType::rpar) {
            syntaxError(sErrors[1] + " in WhileOp -> while (E) Stmt. Expected ')'");
        }
        nextLexem();
        auto l2 = newLabel();
        auto zero = std::make_shared<NumberOperand>(0);
        generateAtom(std::make_unique<ConditionalJumpAtom>("EQ", p, zero, l2), scope);
        Stmt(scope);
        generateAtom(std::make_unique<JumpAtom>(l1), scope);
        generateAtom(std::make_unique<LabelAtom>(l2), scope);
        return;
    }
    syntaxError(sErrors[1] + " in WhileOp -> while (E) Stmt. Expected while");
}

void Translator::ForOp(Scope scope) {
    if (currentLexem_.type() == LexemType::kwfor) {
        if (scope == GlobalScope) {
            syntaxError(sErrors[1] + " in ForOp -> for (ForInit; ForExp; ForLoop) Stmt. " + 
                "'for' must be inside function");
        }
        nextLexem();
        if (currentLexem_.type() == LexemType::eof) {
            syntaxError(sErrors[2] + " in ForOp -> for (ForInit; ForExp; ForLoop) Stmt. " + "Expected '('");
        }
        if (currentLexem_.type() != LexemType::lpar) {
            syntaxError(sErrors[1] + " in ForOp -> for (ForInit; ForExp; ForLoop) Stmt. " + "Expected '('");
        }
        nextLexem();
        ForInit(scope);
        if (currentLexem_.type() == LexemType::eof) {
            syntaxError(sErrors[2] + " in ForOp -> for (ForInit; ForExp; ForLoop) Stmt. " + "Expected ';'");
        }
        if (currentLexem_.type() != LexemType::semicolon) {
            syntaxError(sErrors[1] + " in ForOp -> for (ForInit; ForExp; ForLoop) Stmt. " + "Expected ';'");
        }
        nextLexem();
        auto l1 = newLabel();
        auto l2 = newLabel();
        auto l3 = newLabel();
        auto l4 = newLabel();
        generateAtom(std::make_unique<LabelAtom>(l1), scope);
        auto p = ForExpr(scope);
        if (currentLexem_.type() != LexemType::semicolon) {
            syntaxError("");
        }
        nextLexem();
        auto zero = std::make_shared<NumberOperand>(0);
        generateAtom(std::make_unique<ConditionalJumpAtom>("EQ", p, zero, l4), scope);
        generateAtom(std::make_unique<JumpAtom>(l3), scope);
        generateAtom(std::make_unique<LabelAtom>(l2), scope);
        ForLoop(scope);
        generateAtom(std::make_unique<JumpAtom>(l1), scope);
        if (currentLexem_.type() != LexemType::rpar) {
            syntaxError("");
        }
        nextLexem();
        generateAtom((std::make_unique<LabelAtom>(l3)), scope);
        Stmt(scope);
        generateAtom((std::make_unique<JumpAtom>(l2)), scope);
        generateAtom((std::make_unique<LabelAtom>(l4)), scope);
        return;
    }
    syntaxError("");
}

void Translator::ForInit(Scope scope) {
    if (currentLexem_.type() == LexemType::semicolon) {
        return;
    }
    if (currentLexem_.type() == LexemType::eof) {
        syntaxError(sErrors[2] + " in ForInit -> AssignOrCall. Expected assignment or function call");
    }
    if (currentLexem_.type() != LexemType::id) {
        syntaxError(sErrors[1] + " in ForInit -> AssignOrCall. Expected assignment or function call");
    }
    AssignOrCall(scope);
}

std::shared_ptr<RValue> Translator::ForExpr(Scope scope) {
    if (currentLexem_.type() == LexemType::semicolon) {
        return std::make_shared<NumberOperand>(1);
    }
    return E(scope);
}

void Translator::ForLoop(Scope scope) {
    if (currentLexem_.type() == LexemType::rpar) {
        return;
    }
    if (currentLexem_.type() == LexemType::opinc) {
        nextLexem();
        if (currentLexem_.type() == LexemType::eof) {
            syntaxError(sErrors[2] + "in ForLoop -> ++ id. Expected id");
        }
        if (currentLexem_.type() != LexemType::id) {
            syntaxError(sErrors[1] + "in ForLoop -> ++ id. Expected id");
        }
        std::string name = currentLexem_.str();
        nextLexem();
        auto p = symbolTable_.checkVar(scope, name);
        if (!p) {
            syntaxError(sErrors[1] + "in ForLoop -> ++id. Unknown id: " + name);
        }
        auto one = std::make_shared<NumberOperand>(1);
        generateAtom(std::make_unique<BinaryOpAtom>("ADD", p, one, p), scope);
        return;
    }
    if (currentLexem_.type() == LexemType::id) {
        AssignOrCall(scope);
        return;
    }
    syntaxError(sErrors[1] + " in ForLoop. Expected assignment, function call, or '++ id' in for loop");
}

void Translator::IfOp(Scope scope) {
    if (currentLexem_.type() == LexemType::kwif) {
        if (scope == GlobalScope) {
            syntaxError(sErrors[1] + " in IfOp -> if (E) Stmt ElsePart. 'if' must be inside function");
        }
        nextLexem();
        if (currentLexem_.type() == LexemType::eof) {
            syntaxError(sErrors[2] + " in IfOp -> if (E) Stmt ElsePart. Expected '('");
        }
        if (currentLexem_.type() != LexemType::lpar) {
            syntaxError(sErrors[1] + " in IfOp -> if (E) Stmt ElsePart. Expected '('");
        }
        nextLexem();
        auto p = E(scope);
        if (!p) {
            syntaxError(sErrors[1] + " in IfOp -> if (E) Stmt ElsePart. Expected expression");
        }
        if (currentLexem_.type() == LexemType::eof) {
            syntaxError(sErrors[2] + " in IfOp -> if (E) Stmt ElsePart. Expected ')'");
        }
        if (currentLexem_.type() != LexemType::rpar) {
            syntaxError(sErrors[1] + " in IfOp -> if (E) Stmt ElsePart. Expected ')'");
        }
        nextLexem();
        auto l1 = newLabel();
        auto l2 = newLabel();
        auto zero = std::make_shared<NumberOperand>(0);
        generateAtom(std::make_unique<ConditionalJumpAtom>("EQ", p, zero, l1), scope);
        Stmt(scope);
        generateAtom(std::make_unique<JumpAtom>(l2), scope);
        generateAtom(std::make_unique<LabelAtom>(l1), scope);
        ElsePart(scope);
        generateAtom(std::make_unique<LabelAtom>(l2), scope);
        return;
    }
    syntaxError(sErrors[1] + " in IfOp. Expected 'if'");
}

void Translator::ElsePart(Scope scope) {
    if (currentLexem_.type() == LexemType::kwelse) {
        nextLexem();
        Stmt(scope);
    }
}

void Translator::SwitchOp(Scope scope) {
    if (currentLexem_.type() == LexemType::kwswitch) {
        if (scope == GlobalScope) {
            syntaxError(sErrors[1] + " in SwitchOp -> switch (E) {Cases}. Switch must be inside function");
        }
        nextLexem();
        if (currentLexem_.type() == LexemType::eof) {
            syntaxError(sErrors[2] + " in SwitchOp -> switch (E) {Cases}. Expected '('");
        }
        if (currentLexem_.type() != LexemType::lpar) {
            syntaxError(sErrors[1] + " in SwitchOp -> switch (E) {Cases}. Expected '('");
        }
        nextLexem();
        auto p = E(scope);
        if (!p) {
            syntaxError(sErrors[1] + " in SwitchOp -> switch (E) {Cases}. Expected expression");
        }
        if (currentLexem_.type() == LexemType::eof) {
            syntaxError(sErrors[2] + " in SwitchOp -> switch (E) {Cases}. Expected ')'");
        }
        if (currentLexem_.type() != LexemType::rpar) {
            syntaxError(sErrors[1] + " in SwitchOp -> switch (E) {Cases}. Expected ')'");
        }
        nextLexem();
        if (currentLexem_.type() == LexemType::eof) {
            syntaxError(sErrors[2] + " in SwitchOp -> switch (E) {Cases}. Expected '{'");
        }
        if (currentLexem_.type() != LexemType::lbrace) {
            syntaxError(sErrors[1] + " in SwitchOp -> switch (E) {Cases}. Expected '{'");
        }
        nextLexem();
        auto end = newLabel();
        Cases(p, end, scope);
        if (currentLexem_.type() == LexemType::eof) {
            syntaxError(sErrors[1] + " in SwitchOp -> switch (E) {Cases}. Expected '}'");
        }
        if (currentLexem_.type() != LexemType::rbrace) {
            syntaxError(sErrors[1] + " in SwitchOp -> switch (E) {Cases}. Expected '}'");
        }
        nextLexem();
        generateAtom(std::make_unique<LabelAtom>(end), scope);
        return;
    }
    syntaxError(sErrors[1] + " in SwitchOp: expected 'switch' keyword");
}

void Translator::Cases(std::shared_ptr<RValue> p,
    std::shared_ptr<LabelOperand> end,
    Scope scope) {
    std::shared_ptr<LabelOperand> def = nullptr;
    ACase(p, end, def, scope);
    Cases_(p, end, def, scope);
}

void Translator::Cases_(std::shared_ptr<RValue> p,
    std::shared_ptr<LabelOperand> end,
    std::shared_ptr<LabelOperand>& def,
    Scope scope) {
    if (currentLexem_.type() != LexemType::kwcase &&
        currentLexem_.type() != LexemType::kwdefault) {
        if (def) {
            generateAtom(std::make_unique<JumpAtom>(def), scope);
        } 
        else {
            generateAtom(std::make_unique<JumpAtom>(end), scope);
        }
        return;
    }
    std::shared_ptr<LabelOperand> def1 = nullptr;
    ACase(p, end, def1, scope);
    if (def && def1) {
        syntaxError(sErrors[1] + " in Cases -> ACase Cases_. Two default sections");
    }
    if (!def) {
        def = def1;
    }
    Cases_(p, end, def, scope);
}

void Translator::ACase(std::shared_ptr<RValue> p,
    std::shared_ptr<LabelOperand> end,
    std::shared_ptr<LabelOperand>& def,
    Scope scope) {
    if (currentLexem_.type() == LexemType::kwdefault) {
        nextLexem();
        if (currentLexem_.type() == LexemType::eof) {
            syntaxError(sErrors[2] + " in ACase -> default : StmtList. Expected colon before eof");
        }
        if (currentLexem_.type() != LexemType::colon) {
            syntaxError(sErrors[1] + " in ACase -> default : StmtList. Expected colon after default");
        }
        nextLexem();
        def = newLabel();
        generateAtom(std::make_unique<LabelAtom>(def), scope);
        StmtList(scope);
        generateAtom(std::make_unique<JumpAtom>(end), scope);
        return;
    }
    if (currentLexem_.type() == LexemType::kwcase) {
        nextLexem();
        if (currentLexem_.type() == LexemType::eof) {
            syntaxError(sErrors[2] + " in ACase -> case num : StmtList. Expected constant before eof");
        }
        if (currentLexem_.type() != LexemType::num &&
            currentLexem_.type() != LexemType::chr) {
            syntaxError(sErrors[1] + " in ACase -> case num : StmtList. Expected constant after case");
        }

        auto val = std::make_shared<NumberOperand>(currentLexem_.value());
        nextLexem();
        if (currentLexem_.type() == LexemType::eof) {
            syntaxError(sErrors[2] + " in ACase -> case num : StmtList. Expected ':'");
        }
        auto next = newLabel();
        generateAtom(std::make_unique<ConditionalJumpAtom>("NE", p, val, next), scope);
        if (currentLexem_.type() != LexemType::colon) {
            syntaxError(sErrors[1] + " in ACase -> case num : StmtList. Expected ':'");
        }
        nextLexem();
        StmtList(scope);
        generateAtom(std::make_unique<JumpAtom>(end), scope);
        generateAtom(std::make_unique<LabelAtom>(next), scope);
        def = nullptr;
        return;
    }
    if (currentLexem_.type() == LexemType::eof) {
        syntaxError(sErrors[2] + " in ACase: expected 'case' or 'default'");
    }
    syntaxError(sErrors[1] + " in ACase: expected 'case' or 'default'");
}

void Translator::IOp(Scope scope) {
    if (currentLexem_.type() == LexemType::kwin) {
        nextLexem();
        if (currentLexem_.type() == LexemType::eof) {
            syntaxError(sErrors[2] + " in IOp: expected identifier, found EOF");
        }
        if (currentLexem_.type() != LexemType::id) {
            syntaxError(sErrors[1] + " in IOp: expected variable after 'in'");
        }
        std::string name = currentLexem_.str();
        nextLexem();
        if (currentLexem_.type() == LexemType::eof) {
            syntaxError(sErrors[2] + " in IOp: expected ';' before EOF");
        }
        auto p = symbolTable_.checkVar(scope, name);
        if (!p) {
            syntaxError(sErrors[1] + " in IOp: variable '" + name + "' is not declared");
        }
        generateAtom(std::make_unique<InAtom>(p), scope);
        if (currentLexem_.type() != LexemType::semicolon) {
            syntaxError(sErrors[1] + " in IOp: expected ';' after variable name");
        }
        nextLexem();
        return;
    }
    syntaxError(sErrors[1] + " in IOp");
}

void Translator::OOp(Scope scope) {
    if (currentLexem_.type() == LexemType::kwout) {
        nextLexem();
        OOp_(scope);
        if (currentLexem_.type() == LexemType::eof) {
            syntaxError(sErrors[2] + " in OOp -> out OOp_ ;. Expected ';' before eof");
        }
        if (currentLexem_.type() != LexemType::semicolon) {
            syntaxError(sErrors[1] + " in OOp -> out OOp_ ;. Expected ';' after out");
        }
        nextLexem();
        return;
    }
    syntaxError(sErrors[1] + " in OOp");
}

void Translator::OOp_(Scope scope) {
    if (currentLexem_.type() == LexemType::str) {
        std::string s = currentLexem_.str();
        auto stringId = stringTable_.add(s);
        nextLexem();
        generateAtom(std::make_unique<OutAtom>(stringId), scope);
        return;
    }

    auto p = E(scope);
    if (!p) {
        syntaxError(sErrors[1] + " in OOp_ -> E: expected expression");
    }
    generateAtom(std::make_unique<OutAtom>(p), scope);
}

int Translator::ArgList(Scope scope) {
    if (currentLexem_.type() == LexemType::rpar) {
        return 0;
    }
    auto p = E(scope);
    if (!p) {
        if (currentLexem_.type() == LexemType::eof) {
            syntaxError(sErrors[2] + " in ArgList: expected expression before EOF");
        }
        else {
            syntaxError(sErrors[1] + " in ArgList: expected expression");
        }
    }
    generateAtom(std::make_unique<ParamAtom>(p), scope);
    return ArgList_(scope) + 1;
}

int Translator::ArgList_(Scope scope) {
    if (currentLexem_.type() == LexemType::comma) {
        nextLexem();
        auto p = E(scope);
        if (!p) {
            if (currentLexem_.type() == LexemType::eof) {
                syntaxError(sErrors[2] + " in ArgList_: expected expression before EOF");
            }
            else {
                syntaxError(sErrors[1] + " in ArgList_: expected expression");
            }
        }
        generateAtom(std::make_unique<ParamAtom>(p), scope);
        return ArgList_(scope) + 1;
    }
    return 0;
}

std::shared_ptr<RValue> Translator::E(Scope scope) {
    auto p = E7(scope);
    if (!p) {
        syntaxError(sErrors[1] + " in E -> E7: expected expression");
    }
    return p;
}

std::shared_ptr<RValue> Translator::E7(Scope scope) {
    auto q = E6(scope);
    if (!q) {
        syntaxError(sErrors[1] + " in E7 -> E6 E7': expected left operand");
    }
    auto p = E7_(q, scope);
    if (!p) {
        syntaxError(sErrors[1] + " in E7 -> E6 E7': invalid OR expression");
    }
    return p;
}

std::shared_ptr<RValue> Translator::E7_(std::shared_ptr<RValue> p, Scope scope) {
    if (currentLexem_.type() == LexemType::error) {
        lexicalError(errors[currentLexem_.value()]);
    }

    if (currentLexem_.type() == LexemType::opor) {
        nextLexem();
        if (currentLexem_.type() == LexemType::eof) {
            syntaxError(sErrors[2] + " in E7' -> || E6 E7': unexpected end after '||'");
        }
        auto r = E6(scope);
        if (!r) {
            syntaxError(sErrors[1] + " in E7' -> || E6 E7': expected expression after '||'");
        }
        auto s = symbolTable_.alloc(scope);
        generateAtom(std::make_unique<BinaryOpAtom>("OR", p, r, s), scope);
        return E7_(s, scope);
    }

    return p;
}

std::shared_ptr<RValue> Translator::E6(Scope scope) {
    auto q = E5(scope);
    if (!q) {
        syntaxError(sErrors[1] + " in E6 -> E5 E6': expected expression");
    }

    auto p = E6_(q, scope);
    if (!p) {
        syntaxError(sErrors[1] + " in E6 -> E5 E6': invalid AND expression");
    }

    return p;
}

std::shared_ptr<RValue> Translator::E6_(std::shared_ptr<RValue> p, Scope scope) {
    if (currentLexem_.type() == LexemType::error) {
        lexicalError(errors[currentLexem_.value()]);
    }
    if (currentLexem_.type() == LexemType::opand) {
        nextLexem();
        if (currentLexem_.type() == LexemType::eof) {
            syntaxError(sErrors[2] + "E6' -> && E5 E6': unexpected end after '&&'");
        }
        auto r = E5(scope);
        if (!r) {
            syntaxError(sErrors[1] + " in E6' -> && E5 E6': expected expression after '&&'");
        }
        auto s = symbolTable_.alloc(scope);
        generateAtom(std::make_unique<BinaryOpAtom>("AND", p, r, s), scope);
        return E6_(s, scope);
    }
    return p;
}

std::shared_ptr<RValue> Translator::E5(Scope scope) {
    auto q = E4(scope);
    if (!q) {
        syntaxError(sErrors[1] + " in E5 -> E4 E5': expected expression");
    }
    auto p = E5_(q, scope);
    if (!p) {
        syntaxError(sErrors[1] + " in E5 -> E4 E5': invalid comparison expression");
    }
    return p;
}

std::shared_ptr<RValue> Translator::E5_(std::shared_ptr<RValue> p, Scope scope) {
    if (currentLexem_.type() == LexemType::error) {
        lexicalError(errors[currentLexem_.value()]);
    }
    std::string op;
    if (currentLexem_.type() == LexemType::opeq) op = "EQ";
    else if (currentLexem_.type() == LexemType::opne) op = "NE";
    else if (currentLexem_.type() == LexemType::opgt) op = "GT";
    else if (currentLexem_.type() == LexemType::oplt) op = "LT";
    else if (currentLexem_.type() == LexemType::ople) op = "LE";
    else return p;
    nextLexem();
    if (currentLexem_.type() == LexemType::eof) {
        syntaxError(sErrors[2] + "E5' -> op E4: unexpected end after comparison operator");
    }
    auto r = E4(scope);
    if (!r) { 
        syntaxError(sErrors[1] + " in E5 -> E4 E5': invalid comparison expression");
    }
    auto s = symbolTable_.alloc(scope);
    auto one = std::make_shared<NumberOperand>(1);
    auto zero = std::make_shared<NumberOperand>(0);
    auto l = newLabel();
    generateAtom(std::make_unique<UnaryOpAtom>("MOV", one, s), scope);
    generateAtom(std::make_unique<ConditionalJumpAtom>(op, p, r, l), scope);
    generateAtom(std::make_unique<UnaryOpAtom>("MOV", zero, s), scope);
    generateAtom(std::make_unique<LabelAtom>(l), scope);
    return E5_(s, scope);
}

std::shared_ptr<RValue> Translator::E4(Scope scope) {
    auto r = E3(scope);
    if (!r) {
        syntaxError(sErrors[1] + " in E4 -> E3 E4': expected expression");
    }
    auto s = E4_(r, scope);
    if (!s) {
        syntaxError(sErrors[1] + " in E4 -> E3 E4': invalid addition/subtraction expression");
    }
    return s;
}

std::shared_ptr<RValue> Translator::E4_(std::shared_ptr<RValue> p, Scope scope) {
    if (currentLexem_.type() == LexemType::error) {
        lexicalError(errors[currentLexem_.value()]);
    }
    if (currentLexem_.type() == LexemType::opplus) {
        nextLexem();
        if (currentLexem_.type() == LexemType::eof) {
            syntaxError(sErrors[2] + " in E4' -> + E3 E4': unexpected end after '+'");
        }
        auto r = E3(scope);
        if (!r) {
            syntaxError(sErrors[1] + " in E4' -> + E3 E4': expected expression after '+'");
        }
        auto s = symbolTable_.alloc(scope);
        generateAtom(std::make_unique<BinaryOpAtom>("ADD", p, r, s), scope);
        return E4_(s, scope);
    }
    if (currentLexem_.type() == LexemType::opminus) {
        nextLexem();
        if (currentLexem_.type() == LexemType::eof) {
            syntaxError(sErrors[2] + " in E4' -> - E3 E4': unexpected end after '-'");
        }
        auto r = E3(scope);
        if (!r) {
            syntaxError(sErrors[1] + " in E4' -> - E3 E4': expected expression after '-'");
        }
        auto s = symbolTable_.alloc(scope);
        generateAtom(std::make_unique<BinaryOpAtom>("SUB", p, r, s), scope);
        return E4_(s, scope);
    }
    return p;
}

std::shared_ptr<RValue> Translator::E3(Scope scope) {
    auto r = E2(scope);
    if (!r) {
        syntaxError(sErrors[1] + " in E3 -> E2 E3': expected expression");
    }
    auto s = E3_(r, scope);
    if (!s) {
        syntaxError(sErrors[1] + " in E3 -> E2 E3': invalid multiplication expression");
    }
    return s;
}

std::shared_ptr<RValue> Translator::E3_(std::shared_ptr<RValue> p, Scope scope) {
    if (currentLexem_.type() == LexemType::error) {
        lexicalError(errors[currentLexem_.value()]);
    }
    if (currentLexem_.type() == LexemType::opmult) {
        nextLexem();
        if (currentLexem_.type() == LexemType::eof) {
            syntaxError(sErrors[2] + " in E3' -> * E2 E3': unexpected end after '*'");
        }
        auto r = E2(scope);
        if (!r) {
            syntaxError(sErrors[1] + " in E3' -> * E2 E3': expected expression after '*'");
        }
        auto s = symbolTable_.alloc(scope);
        generateAtom(std::make_unique<BinaryOpAtom>("MUL", p, r, s), scope);
        return E3_(s, scope);
    }
    return p;
}

std::shared_ptr<RValue> Translator::E2(Scope scope) {
    if (currentLexem_.type() == LexemType::error) {
        lexicalError(errors[currentLexem_.value()]);
    }
    if (currentLexem_.type() == LexemType::opnot) {
        nextLexem();
        if (currentLexem_.type() == LexemType::eof) {
            syntaxError(sErrors[2] + " in E2 -> ! E1: unexpected end after '!'");
        }
        auto q = E1(scope);
        if (!q) {
            syntaxError(sErrors[1] + " in E2 -> ! E1: expected expression after '!'");
        }
        auto r = symbolTable_.alloc(scope);
        generateAtom(std::make_unique<UnaryOpAtom>("NOT",q, r), scope);
        return r;
    }
    return E1(scope);
}

std::shared_ptr<RValue> Translator::E1(Scope scope) {
    if (currentLexem_.type() == LexemType::error) {
        lexicalError(errors[currentLexem_.value()]);
    }
    if (currentLexem_.type() == LexemType::opinc) {
        nextLexem();
        if (currentLexem_.type() == LexemType::eof) {
            syntaxError(sErrors[2] + " in E1 -> ++ id: unexpected end after '++'");
        }
        if (currentLexem_.type() != LexemType::id) {
            syntaxError(sErrors[1] + " in E1 -> ++ id: expected identifier after '++'");
        }
        auto p = symbolTable_.checkVar(scope, currentLexem_.str());
        if (!p) {
            syntaxError("");
        }
        auto one = std::make_shared<NumberOperand>(1);
        generateAtom(std::make_unique<BinaryOpAtom>("ADD", p, one, p), scope);
        nextLexem();
        return p;
    }
    if (currentLexem_.type() == LexemType::lpar) {
        nextLexem();
        if (currentLexem_.type() == LexemType::eof) {
            syntaxError(sErrors[2] + " in E1 -> (E): unexpected end, expected ')'");
        }
        auto q = E(scope);
        if (!q) {
            syntaxError(sErrors[1] + " in E1 -> (E): invalid expression inside parentheses");
        }
        if (currentLexem_.type() != LexemType::rpar) {
            syntaxError(sErrors[1] + " in E1 -> (E): expected ')'");
        }
        nextLexem();
        return q;
    }

    if (currentLexem_.type() == LexemType::num) {
        auto p = std::make_shared<NumberOperand>(currentLexem_.value());
        nextLexem();
        return p;
    }

    if (currentLexem_.type() == LexemType::chr) {
        auto p = std::make_shared<NumberOperand>(currentLexem_.value());
        nextLexem();
        return p;
    }

    if (currentLexem_.type() == LexemType::id) {
        std::string name = currentLexem_.str();
        nextLexem();
        return E1_(name, scope);
    }
    syntaxError(sErrors[1] + " in E1: unexpected token, expected expression");
    return nullptr;
}

std::shared_ptr<RValue> Translator::E1_(const std::string& p, Scope scope) {
    if (currentLexem_.type() == LexemType::error) {
        lexicalError(errors[currentLexem_.value()]);
    }
    if (currentLexem_.type() == LexemType::opinc) {
        nextLexem();
        auto s = symbolTable_.checkVar(scope, p);
        if (!s) {
            syntaxError(sErrors[1] + " in E_ -> ++: Undefined variable: " + p);
        }
        auto r = symbolTable_.alloc(scope);
        auto one = std::make_shared<NumberOperand>(1);
        generateAtom(std::make_unique<UnaryOpAtom>("MOV", s, r), scope);
        generateAtom(std::make_unique<BinaryOpAtom>("ADD", s, one, s), scope);
        return r;
    } 
    if (currentLexem_.type() == LexemType::lpar) {
        nextLexem();
        int n = ArgList(scope);
        if (currentLexem_.type() != LexemType::rpar) {
            syntaxError(sErrors[1] + " in E_ -> (ArgList): Expected ')' after function arguments");
        }
        nextLexem();
        auto s = symbolTable_.checkFunc(p, n);
        if (!s) syntaxError(sErrors[1] + " in E_ -> (ArgList): Undefined function or wrong argument count: " + p);

        auto r = symbolTable_.alloc(scope);
        generateAtom(std::make_unique<CallAtom>(s, r), scope);
        return r;
    }
    auto q = symbolTable_.checkVar(scope, p);
    if (!q) {
        syntaxError(sErrors[1] + " in E_ -> epsilon: undefined variable " + p);
    }
    return q;
}

const SymbolTable& Translator::getSymbolTable() const {
    return symbolTable_;
}

const StringTable& Translator::getStringTable() const {
    return stringTable_;
}

std::map<int, std::string> sErrors{
    {1, "UnexpectedToken"},
    {2, "UnexpectedEOF"},
    {3, "ExtraTokens"}
};
