
#include "FileClasses.h"
#include "Directory.h"
#include <iostream>
#include <limits>

// ------------------- SingleLineFile -------------------
SingleLineFile::SingleLineFile(const std::string& name, Directory* parent)
    : Entry(name, parent), content("") {}

std::string SingleLineFile::displayName() const {
    return name;
}

void SingleLineFile::edit() {
    std::cout << "Editing single-line file '" << name << "'.\n";
    std::cout << "Current content: " << content << "\n";
    std::cout << "Enter new content: ";
    std::string newContent;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // очистка буфера
    std::getline(std::cin, newContent);
    content = newContent;
    std::cout << "File updated.\n";
}

void SingleLineFile::display() const {
    std::cout << content << std::endl;
}

// ------------------- MultiLineFile -------------------
MultiLineFile::MultiLineFile(const std::string& name, Directory* parent)
    : Entry(name, parent) {}

std::string MultiLineFile::displayName() const {
    return name;
}

void MultiLineFile::edit() {
    std::cout << "Editing multi-line file '" << name << "'.\n";
    if (!lines.empty()) {
        std::cout << "Current lines:\n";
        for (size_t i = 0; i < lines.size(); ++i) {
            std::cout << (i + 1) << ": " << lines[i] << '\n';
        }
    }

    std::cout << "Enter line number to edit (1-" << lines.size() << ")\n";
    std::cout << "or 0 to add new lines (input -1 to stop): ";
    int choice;
    std::cin >> choice;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    if (choice > 0 && choice <= static_cast<int>(lines.size())) {
        std::cout << "Enter new content for line " << choice << ": ";
        std::string newLine;
        std::getline(std::cin, newLine);
        lines[choice - 1] = newLine;
        std::cout << "Line updated.\n";
    } else if (choice == 0) {
        std::cout << "Adding new lines. Enter text line by line, type '-1' to finish.\n";
        std::string line;
        while (true) {
            std::getline(std::cin, line);
            if (line == "-1") break;
            lines.push_back(line);
        }
        std::cout << lines.size() << " line(s) total.\n";
    } else {
        std::cout << "Invalid line number.\n";
    }
}

void MultiLineFile::display() const {
    if (lines.empty()) {
        std::cout << "(empty)\n";
    } else {
        for (size_t i = 0; i < lines.size(); ++i) {
            std::cout << (i + 1) << ": " << lines[i] << '\n';
        }
    }
}

// ------------------- PostponedFile -------------------
PostponedFile::PostponedFile(const std::string& name, Directory* parent)
    : Entry(name, parent) {}

std::string PostponedFile::displayName() const {
    return name;
}

void PostponedFile::edit() {
    std::cout << "File '" << name << "' has no type yet.\n";
    std::cout << "Choose type: (s)ingle-line or (m)ulti-line: ";
    char type;
    std::cin >> type;
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    Directory* dir = getParent();
    if (!dir) {
        std::cout << "Error: orphan file.\n";
        return;
    }

    Entry* newFile = nullptr;
    if (type == 's' || type == 'S') {
        newFile = new SingleLineFile(name);
    } else if (type == 'm' || type == 'M') {
        newFile = new MultiLineFile(name);
    } else {
        std::cout << "Invalid choice, keeping as postponed.\n";
        return;
    }

    dir->replaceEntry(this, newFile); // заменяет this на newFile и удаляет this
    newFile->edit();                  // сразу начинаем редактирование
}

void PostponedFile::display() const {
    std::cout << "Empty postponed file (type not set).\n";
}
