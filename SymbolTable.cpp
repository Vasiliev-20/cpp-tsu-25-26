#include "SymbolTable.h"
#include "Atoms.h"
#include <algorithm>

const SymbolTable::TableRecord& SymbolTable::operator[](int index) const {  //	Доступ к записи по индексу (только чтение)
    return _records[index];
}

std::shared_ptr<MemoryOperand> SymbolTable::add(const std::string& name) {   // Добавляет переменную или возвращает существующую + кэширует MemoryOperand
    auto it = std::find_if(_records.begin(), _records.end(),
        [&name](const TableRecord& rec) { return rec._name == name; });
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
    int newIndex = static_cast<int>(_records.size());
    _records.push_back(TableRecord(name));
    auto operand = std::make_shared<MemoryOperand>(newIndex, this);
    _operands.push_back(operand);
    return operand;
}

std::ostream& operator<<(std::ostream& os, const SymbolTable& st) {    Вывод таблицы для отладки
    for (size_t i = 0; i < st._records.size(); ++i) {
        os << i << " " << st._records[i]._name << std::endl;
    }
    return os;
}

std::shared_ptr<MemoryOperand> SymbolTable::alloc() {  Генерирует уникальное временное имя __tmpX и добавляет его в таблицу
    static int counter = 0;
    std::string tempName = "__tmp" + std::to_string(++counter);
    return add(tempName);
}
