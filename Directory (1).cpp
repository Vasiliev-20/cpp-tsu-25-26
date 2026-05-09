
#include "Directory.h"
#include <iostream>
#include <algorithm>

Directory::Directory(const std::string& name, Directory* parent)
    : Entry(name, parent) {}

Directory::~Directory() {
    for (Entry* child : children) {
        delete child;
    }
}

std::string Directory::displayName() const {
    return name + "/";
}

std::vector<Entry*> Directory::listAll() const {
    return children; // копия вектора указателей
}

Directory* Directory::asDirectory() {
    return this;
}

void Directory::edit() {
    std::cout << "Cannot edit a directory.\n";
}

void Directory::hide() {
    Entry::hide();               // скрыть своё имя
    for (Entry* child : children) {
        child->hide();           // рекурсивно скрыть потомков
    }
}

void Directory::restore() {
    Entry::restore();
    for (Entry* child : children) {
        child->restore();
    }
}

void Directory::display() const {
    std::cout << "Is a directory.\n";
}

void Directory::addEntry(Entry* entry) {
    children.push_back(entry);
    entry->setParent(this);
}

void Directory::replaceEntry(Entry* oldEntry, Entry* newEntry) {
    auto it = std::find(children.begin(), children.end(), oldEntry);
    if (it != children.end()) {
        *it = newEntry;
        newEntry->setParent(this);
        delete oldEntry;
    }
}

Entry* Directory::find(const std::string& name) const {
    for (Entry* child : children) {
        if (child->getName() == name) {
            return child;
        }
    }
    return nullptr;
}
