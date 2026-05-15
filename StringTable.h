#ifndef STRINGTABLE_H
#define STRINGTABLE_H

#include <string>
#include <vector>
#include <iostream>
#include <memory> // для std::shared_ptr

class StringOperand;  // предварительное объявление

class StringTable {
protected:
    std::vector<std::string> _strings;
    std::vector<std::shared_ptr<StringOperand>> _operands;
public:
    const std::string& operator[](int index) const;
    std::shared_ptr<StringOperand> add(const std::string& name);
    friend std::ostream& operator<<(std::ostream& os, const StringTable& st);
};

#endif
