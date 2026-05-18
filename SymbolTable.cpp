#include "SymbolTable.h"
#include "Atoms.h"
#include <algorithm>

const SymbolTable::TableRecord& SymbolTable::operator[](int index) const {  //	Доступ к записи по индексу (только чтение)
    return _records[index];
}

    auto it = std::find_if(_records.begin(), _records.end(),
        [&](const TableRecord& rec) {
            return rec._name == name && rec._kind == kind && rec._scope == scope;
        });
    if (it != _records.end()) {
        int index = static_cast<int>(it - _records.begin());
        if (index < static_cast<int>(_operands.size()) && _operands[index]) {
            return _operands[index];
        }
        auto operand = std::make_shared<MemoryOperand>(index, this);
        if (index >= static_cast<int>(_operands.size()))
            _operands.resize(index + 1);
        _operands[index] = operand;
        return operand;
    }
    // Новая запись
    int newIndex = static_cast<int>(_records.size());
    _records.push_back(TableRecord(name));
    auto& rec = _records.back();
    rec._kind = kind;
    rec._type = type;
    rec._len = len;
    rec._init = init;
    rec._scope = scope;
    rec._offset = offset;

    auto operand = std::make_shared<MemoryOperand>(newIndex, this);
    _operands.push_back(operand);
    return operand;
}

std::shared_ptr<MemoryOperand> SymbolTable::alloc() {  // Генерирует уникальное временное имя __tmpX и добавляет его в таблицу
    static int counter = 0;
    std::string tempName = "__tmp" + std::to_string(++counter);
    return add(tempName, RecordKind::var, RecordType::integer, -1, 0, GlobalScope, -1);
}

int SymbolTable::find(const std::string& name, Scope scope) const {
    // Сначала ищем в локальной области, потом в глобальной
    for (int i = static_cast<int>(_records.size()) - 1; i >= 0; --i) {
        const auto& rec = _records[i];
        if (rec._name == name && rec._scope == scope)
            return i;
    }
    if (scope != GlobalScope) {
        for (int i = static_cast<int>(_records.size()) - 1; i >= 0; --i) {
            const auto& rec = _records[i];
            if (rec._name == name && rec._scope == GlobalScope)
                return i;
        }
    }
    return -1;
}

//  // // // // // // // ^^^^^^
std::ostream& operator<<(std::ostream& os, const SymbolTable& st) {
    // Заголовок как в примере
    os << "SYMBOL TABLE\n";
    os << "-------------------------------------------------------\n";
    os << std::left << std::setw(6) << "code"
       << std::setw(12) << "name"
       << std::setw(8) << "kind"
       << std::setw(8) << "type"
       << std::setw(6) << "len"
       << std::setw(8) << "init"
       << std::setw(8) << "scope"
       << std::setw(8) << "offset\n";
    for (size_t i = 0; i < st._records.size(); ++i) {
        const auto& rec = st._records[i];
        os << std::left << std::setw(6) << i
           << std::setw(12) << rec._name
           << std::setw(8);
        switch (rec._kind) {
            case SymbolTable::RecordKind::var: os << "var"; break;
            case SymbolTable::RecordKind::func: os << "func"; break;
            default: os << "unknown";
        }
        os << std::setw(8);
        switch (rec._type) {
            case SymbolTable::RecordType::integer: os << "int"; break;
            case SymbolTable::RecordType::chr: os << "chr"; break;
            default: os << "unknown";
        }
        os << std::setw(6) << (rec._len == -1 ? "" : std::to_string(rec._len))
           << std::setw(8) << (rec._init == 0 && rec._kind == SymbolTable::RecordKind::var ? "" : std::to_string(rec._init))
           << std::setw(8) << rec._scope
           << std::setw(8) << rec._offset
           << std::endl;
    }
    return os;
}
