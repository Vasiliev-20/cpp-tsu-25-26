#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H

#include <string>
#include <vector>
#include <iostream>
#include <memory>          // для std::shared_ptr

// Forward declaration (чтобы не включать Atoms.h пока в заголовке)
class MemoryOperand;

class SymbolTable {
public:
    std::shared_ptr<MemoryOperand> alloc();
    struct TableRecord {
        std::string _name;
        TableRecord(const std::string& name = "") : _name(name) {}
        bool operator==(const TableRecord& other) const {  // Перегрузка
            return _name == other._name;
        }
    };

protected:
    std::vector<TableRecord> _records;
    // Для кэширования созданных операндов (чтобы для одной записи не создавать много shared_ptr)
    std::vector<std::shared_ptr<MemoryOperand>> _operands;

public:
    const TableRecord& operator[](int index) const;
    // Новый метод add, возвращающий shared_ptr<MemoryOperand>
    std::shared_ptr<MemoryOperand> add(const std::string& name);
    friend std::ostream& operator<<(std::ostream& os, const SymbolTable& st);
};

#endif
