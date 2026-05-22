#pragma once
#include "Atoms.h"
#include <iostream>
#include <vector>
#include <stdexcept>
#include <algorithm>
#include <memory>
#include <iomanip>

class StringTable {
public:
    const std::string& operator[](const int index) const;
    std::shared_ptr<StringOperand> add(const std::string& str);
    friend std::ostream& operator<<(std::ostream& os, const StringTable& table);
    void generateStrings(std::ostream& stream) const;
protected:
    std::vector<std::string> strings_;
};
