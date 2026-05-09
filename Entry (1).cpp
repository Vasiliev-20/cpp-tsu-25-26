
#include "Entry.h"
#include <iostream>

Entry::Entry(const std::string& name, Directory* parent)
    : name(name), parent(parent) {}

std::vector<Entry*> Entry::listAll() const {
    return {}; // по умолчанию пустой список (для файлов)
}

Directory* Entry::asDirectory() {
    return nullptr;
}

void Entry::edit() {
    std::cout << "Cannot edit this entry.\n";
}

void Entry::hide() {
    if (!isHidden()) {
        name = "." + name;
    }
}

void Entry::restore() {
    if (isHidden() && name.length() > 1) {
        name = name.substr(1); // убираем первую точку
    }
}

bool Entry::isHidden() const {
    return !name.empty() && name[0] == '.';
}

const std::string& Entry::getName() const {
    return name;
}

void Entry::setName(const std::string& newName) {
    name = newName;
}

Directory* Entry::getParent() const {
    return parent;
}

void Entry::setParent(Directory* p) {
    parent = p;
}
