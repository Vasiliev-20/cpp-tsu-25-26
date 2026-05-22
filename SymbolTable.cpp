#include "SymbolTable.h"

bool SymbolTable::TableRecord::operator==(const TableRecord& other) const {
    return name_ == other.name_;
}

const SymbolTable::TableRecord& SymbolTable::operator[](const int index) const {
    if (index < 0 || (size_t)index >= records_.size()) {
        throw std::out_of_range("Index is out of range");
    }
    return records_[index];
}

//std::shared_ptr<MemoryOperand> SymbolTable::add(const std::string& name) {
//    TableRecord target{ name };
//    auto it = std::find(records_.begin(), records_.end(), target);
//    int index;
//    if (it != records_.end()) {
//        index =  static_cast<int>(std::distance(records_.begin(), it));
//    }
//    else {
//        records_.push_back(target);
//        index = static_cast<int>(records_.size() - 1);
//    }
//    return std::make_shared<MemoryOperand>(index, this);
//}

std::shared_ptr<MemoryOperand> 
SymbolTable::addVar(
    const std::string& name,
    const Scope scope,
    const TableRecord::RecordType type,
    const int init) {
    for (const auto& rec : records_) {
        if (rec.name_ == name && rec.scope_ == scope) {
            return nullptr;
        }
    }
    TableRecord rec;
    rec.name_ = name;
    rec.kind_ = TableRecord::RecordKind::var;
    rec.type_ = type;
    rec.init_ = init;
    rec.scope_ = scope;

    records_.push_back(rec);
    return std::make_shared<MemoryOperand>(records_.size() - 1, this);
}

std::shared_ptr<MemoryOperand>
SymbolTable::addFunc(
    const std::string& name,
    const TableRecord::RecordType type,
    const int len)
{
    for (const auto& rec : records_) {
        if (rec.name_ == name)
            return nullptr;
    }

    TableRecord rec;
    rec.name_ = name;
    rec.kind_ = TableRecord::RecordKind::func;
    rec.type_ = type;
    rec.len_ = len;
    rec.scope_ = GlobalScope;

    records_.push_back(rec);

    return std::make_shared<MemoryOperand>(records_.size() - 1, this);
}

std::shared_ptr<MemoryOperand>
SymbolTable::checkVar(
    const Scope scope,
    const std::string& name)
{
    for (int i = 0; i < records_.size(); i++) {
        if (records_[i].name_ == name &&
            records_[i].scope_ == scope)
        {
            if (records_[i].kind_ ==
                TableRecord::RecordKind::var)
            {
                return std::make_shared<MemoryOperand>(i, this);
            }
            return nullptr;
        }
    }

    if (scope != GlobalScope) {
        for (int i = 0; i < records_.size(); i++) {
            if (records_[i].name_ == name &&
                records_[i].scope_ == GlobalScope)
            {
                if (records_[i].kind_ ==
                    TableRecord::RecordKind::var)
                {
                    return std::make_shared<MemoryOperand>(i, this);
                }
                return nullptr;
            }
        }
    }

    return nullptr;
}

std::shared_ptr<MemoryOperand>
SymbolTable::checkFunc(
    const std::string& name,
    int len)
{
    for (int i = 0; i < records_.size(); i++) {
        if (records_[i].name_ == name &&
            records_[i].scope_ == GlobalScope &&
            records_[i].kind_ ==
            TableRecord::RecordKind::func)
        {
            if (records_[i].len_ == len) {
                return std::make_shared<MemoryOperand>(i, this);
            }
            return nullptr;
        }
    }
    return nullptr;
}

std::shared_ptr<MemoryOperand> SymbolTable::alloc(Scope scope) {
    std::string temp = "[tmp" + std::to_string(tempNum_) + "]";
    ++tempNum_;
    return addVar(
        temp,
        scope,
        TableRecord::RecordType::integer,
        0
    );
}

void SymbolTable::updateFuncLen(int index, int len) {
    if (index >= 0 && static_cast<size_t>(index) < records_.size()) {
        if (records_[index].kind_ == TableRecord::RecordKind::func) {
            records_[index].len_ = len;
        }
    }
}

static std::map<SymbolTable::TableRecord::RecordKind, std::string> kindToStr{
            {SymbolTable::TableRecord::RecordKind::unknown, "unknown"},
            {SymbolTable::TableRecord::RecordKind::var, "var"},
            {SymbolTable::TableRecord::RecordKind::func, "func"}
};
static std::map<SymbolTable::TableRecord::RecordType, std::string> typeToStr{
    {SymbolTable::TableRecord::RecordType::unknown, "unknown"},
    {SymbolTable::TableRecord::RecordType::integer, "integer"},
    {SymbolTable::TableRecord::RecordType::chr, "chr"}
};

int SymbolTable::getM(int scope) const {
    int count = 0;
    for (const auto& rec : records_) {
        if (rec.scope_ == scope && rec.kind_ == TableRecord::RecordKind::var) {
            count++;
        }
    }
    if (scope >= 0 && scope < static_cast<int>(records_.size())) {
        count -= records_[scope].len_;
    }
    return count;
}

int SymbolTable::getReturnOffset(int scope) const {
    int n = records_[scope].len_;
    int m = getM(scope);
    return 2 * (m + n + 1);
}

void SymbolTable::calculateOffset() {
    std::map<Scope, int> scopeCounters;
    for (auto& rec : records_) {
        if (rec.kind_ == TableRecord::RecordKind::func) {
            rec.offset_ = -1;
            continue;
        }
        if (rec.kind_ == TableRecord::RecordKind::var) {
            if (rec.scope_ == GlobalScope) {
                rec.offset_ = -1;
                continue;
            }
            Scope currentScope = rec.scope_;
            scopeCounters[currentScope]++;
            int i = scopeCounters[currentScope];
            int n = records_[currentScope].len_;
            int m = getM(currentScope);
            if (i <= n) {
                rec.offset_ = 2 * (m + n + 1 - i);
            }
            else {
                rec.offset_ = 2 * (m + n - i);
            }
        }
    }
}

void SymbolTable::generateGlobals(std::ostream & stream) const {
    for (int i = 0; const auto& rec : records_) {
        if (rec.kind_ == TableRecord::RecordKind::var 
            && rec.scope_ == GlobalScope) {
            stream << "var" << i << ": " << "DB " << rec.init_ << "\n";
        }
        i++;
    }
}

size_t SymbolTable::size() {
    return records_.size();
}

void SymbolTable::print(std::ostream& os) const {
    os << "SYMBOL TABLE" << std::endl;
    os << std::setfill('-') << std::setw(75) << "-" << std::endl;
    os << std::setfill(' ') << std::left;
    os << std::setw(8) << "code"
        << std::setw(12) << "name"
        << std::setw(8) << "kind"
        << std::setw(8) << "type"
        << std::setw(8) << "len"
        << std::setw(8) << "init"
        << std::setw(8) << "scope"
        << std::setw(8) << "offset" << std::endl;
    for (int i = 0; i < records_.size(); ++i) {
        const auto& rec = records_[i];
        os << std::setw(8) << i
            << std::setw(12) << rec.name_
            << std::setw(8) << kindToStr.at(rec.kind_)
            << std::setw(8) << typeToStr.at(rec.type_);
        if (rec.kind_ == TableRecord::RecordKind::func) {
            os << std::setw(8) << rec.len_;
        }
        else {
            os << std::setw(8) << "None";
        }
        os << std::setw(8) << rec.init_
            << std::setw(8) << rec.scope_
            << std::setw(8) << rec.offset_
            << std::endl;
    }
}mbolTable::alloc() { // Генерирует уникальное временное имя __tmpX и добавляет его в таблицу
    static int counter = 0;
    std::string tempName = "__tmp" + std::to_string(++counter);
    return add(tempName);
}
