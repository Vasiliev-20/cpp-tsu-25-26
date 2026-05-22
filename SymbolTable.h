#pragma once
#include "Atoms.h"
#include <memory>
#include <iostream>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <iomanip>
#include <map>
using Scope = int;
const Scope GlobalScope = -1;

class SymbolTable {
public:
    struct TableRecord {
        enum class RecordKind {
            unknown,
            var,
            func
        };
        enum class RecordType {
            unknown,
            integer,
            chr
        };
        std::string name_;
        std::string name() const { return name_; }
        RecordKind kind_ = RecordKind::unknown;
        RecordType type_ = RecordType::unknown;
        int len_ = -1;
        int init_ = 0;
        Scope scope_ = GlobalScope;
        int offset_ = -1;
        bool operator == (const TableRecord& other) const;
        
    };
    const TableRecord& operator[](const int index) const;
    //std::shared_ptr<MemoryOperand> add(const std::string& name);
    std::shared_ptr<MemoryOperand> addVar(const std::string& name,
        const Scope scope,
        const TableRecord::RecordType type,
        const int init = 0);
    std::shared_ptr<MemoryOperand> addFunc(const std::string& name,
            const TableRecord::RecordType type,
            const int len);
    std::shared_ptr<MemoryOperand> checkVar(const Scope scope,
            const std::string& name);
    std::shared_ptr<MemoryOperand> checkFunc(const std::string& name, int len);
    void updateFuncLen(int index, int len);
    std::shared_ptr<MemoryOperand> alloc(Scope scope);
    void print(std::ostream& os) const;
    int getM(int scope) const;
    void calculateOffset();
    int getReturnOffset(int scope) const;
    void generateGlobals(std::ostream& stream) const;
    size_t size();
protected:
    std::vector<TableRecord> records_;
    size_t tempNum_ = 1;
};
