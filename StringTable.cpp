#include "StringTable.h"
#include "Atoms.h"
#include <algorithm>

const std::string& StringTable::operator[](int index) const {
    return _strings[index];
}

std::shared_ptr<StringOperand> StringTable::add(const std::string& name) {
    auto it = std::find(_strings.begin(), _strings.end(), name);
    if (it != _strings.end()) {
        int index = static_cast<int>(it - _strings.begin());
        if (index < static_cast<int>(_operands.size()) && _operands[index]) {
            return _operands[index];
        }
        auto operand = std::make_shared<StringOperand>(index, this);
        if (index >= static_cast<int>(_operands.size()))
            _operands.resize(index + 1);
        _operands[index] = operand;
        return operand;
    }
    int newIndex = static_cast<int>(_strings.size());
    _strings.push_back(name);
    auto operand = std::make_shared<StringOperand>(newIndex, this);
    _operands.push_back(operand);
    return operand;
}

std::ostream& operator<<(std::ostream& os, const StringTable& st) {
    for (size_t i = 0; i < st._strings.size(); ++i) {
        os << i << " " << st._strings[i] << std::endl;
    }
    return os;
}
