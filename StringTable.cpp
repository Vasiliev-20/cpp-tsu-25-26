#include "StringTable.h"

const std::string& StringTable::operator[](const int index) const {
    if (index < 0 || index >= strings_.size()) {
        throw std::out_of_range("Index is out of range");
    }
    return strings_[index];
}
std::shared_ptr<StringOperand> StringTable::add(const std::string& name) {
    auto it = std::find(strings_.begin(), strings_.end(), name);
    int index;
    if (it != strings_.end()) {
        index = std::distance(strings_.begin(), it);
    }
    else {
        strings_.push_back(name);
        index = strings_.size() - 1;
    }
    return std::make_shared<StringOperand>(index, this);
}

void StringTable::generateStrings(std::ostream& stream) const {
    for (int i = 0; const auto& str : strings_) {
        stream << "str" << i << ": " << "DB " << str << ", 0\n";
        ++i;
    }
}

std::ostream& operator<<(std::ostream& os, const StringTable& table) {
    os << "STRING TABLE" << std::endl;
    os << std::setfill('-') << std::setw(50) << "-" << std::endl;
    os << std::setfill(' ');
    for (size_t i = 0; i < table.strings_.size(); ++i) {
        os << std::left << std::setw(8) << i << std::setw(8) << table.strings_[i] << std::endl;
    }
    os << std::endl;
    return os;
}
