#include "Terminal.h"
#include "Directory.h"
#include "FileClasses.h"
#include <iostream>
#include <sstream>

Terminal::Terminal() {
    root = new Directory("/");
    path.push_back(root);
}

Terminal::~Terminal() {
    delete root; // рекурсивно удалит всё дерево
}

std::string Terminal::prompt() const {
    if (path.empty()) return "? ";
    std::string p = "/";
    // root всегда первый, пропускаем его, чтобы избежать двойного слеша
    for (size_t i = 1; i < path.size(); ++i) {
        p += path[i]->getName() + "/";
    }
    return p + "$ ";
}

void Terminal::cmdLs() {
    Directory* current = path.back();
    auto entries = current->listAll();
    bool found = false;
    for (Entry* e : entries) {
        if (!e->isHidden()) {
            std::cout << e->displayName() << "  ";
            found = true;
        }
    }
    if (!found) std::cout << "(empty)";
    std::cout << std::endl;
}

void Terminal::cmdCd(const std::string& arg) {
    if (arg == "..") {
        if (path.size() > 1) {
            path.pop_back();
        } else {
            std::cout << "Already at root.\n";
        }
        return;
    }

    Directory* current = path.back();
    Entry* entry = current->find(arg);
    if (!entry) {
        std::cout << "cd: no such file or directory: " << arg << "\n";
        return;
    }
    Directory* dir = entry->asDirectory();
    if (!dir) {
        std::cout << "cd: not a directory: " << arg << "\n";
        return;
    }
    path.push_back(dir);
}

void Terminal::cmdCat(const std::string& arg) {
    Directory* current = path.back();
    Entry* entry = current->find(arg);
    if (!entry) {
        std::cout << "cat: " << arg << ": No such file\n";
        return;
    }
    entry->display();
}

void Terminal::cmdTouch(const std::string& arg) {
    Directory* current = path.back();
    if (current->find(arg)) {
        std::cout << "touch: " << arg << ": File already exists\n";
        return;
    }
    current->addEntry(new PostponedFile(arg));
    std::cout << "Created file '" << arg << "' (type postponed).\n";
}

void Terminal::cmdMkdir(const std::string& arg) {
    Directory* current = path.back();
    if (current->find(arg)) {
        std::cout << "mkdir: " << arg << ": File exists\n";
        return;
    }
    current->addEntry(new Directory(arg));
    std::cout << "Directory '" << arg << "' created.\n";
}

void Terminal::cmdRm(const std::string& arg) {
    Directory* current = path.back();
    Entry* entry = current->find(arg);
    if (!entry) {
        std::cout << "rm: " << arg << ": No such file\n";
        return;
    }
    entry->hide();
    std::cout << "'" << arg << "' is now hidden (soft deleted).\n";
}

void Terminal::cmdRestore(const std::string& arg) {
    // для восстановления нужно заглянуть и в скрытые файлы
    Directory* current = path.back();
    auto entries = current->listAll();
    Entry* found = nullptr;
    for (Entry* e : entries) {
        if (e->getName() == arg) { // точное совпадение имени со скрытой точкой
            found = e;
            break;
        }
    }
    if (!found) {
        std::cout << "restore: " << arg << ": No such file\n";
        return;
    }
    if (!found->isHidden()) {
        std::cout << "'" << arg << "' is not hidden.\n";
        return;
    }
    found->restore();
    std::cout << "'" << arg << "' restored.\n";
}

void Terminal::cmdMv(const std::string& oldName, const std::string& newName) {
    Directory* current = path.back();
    Entry* entry = current->find(oldName);
    if (!entry) {
        std::cout << "mv: " << oldName << ": No such file\n";
        return;
    }
    if (current->find(newName)) {
        std::cout << "mv: " << newName << ": Already exists\n";
        return;
    }
    entry->setName(newName);
    std::cout << "Renamed '" << oldName << "' to '" << newName << "'.\n";
}

void Terminal::cmdEdit(const std::string& arg) {
    Directory* current = path.back();
    Entry* entry = current->find(arg);
    if (!entry) {
        std::cout << "edit: " << arg << ": No such file\n";
        return;
    }
    entry->edit(); // полиморфный вызов
}

void Terminal::cmdHelp() const {
    std::cout << "Available commands:\n"
              << "  ls                    - list directory contents\n"
              << "  cd <dir>             - change directory\n"
              << "  cd ..                - go up\n"
              << "  cat <file>           - display file content\n"
              << "  touch <file>         - create new file (postponed)\n"
              << "  mkdir <dir>          - create directory\n"
              << "  rm <name>            - soft delete (hide)\n"
              << "  restore <name>       - restore hidden item\n"
              << "  mv <old> <new>       - rename\n"
              << "  edit <file>          - edit file\n"
              << "  help                 - this message\n"
              << "  exit                 - quit\n";
}

void Terminal::run() {
    std::string input;
    while (true) {
        std::cout << prompt();
        if (!std::getline(std::cin, input)) break; // EOF
        if (input.empty()) continue;

        std::istringstream iss(input);
        std::string cmd;
        iss >> cmd;

        if (cmd == "exit") {
            break;
        } else if (cmd == "help") {
            cmdHelp();
        } else if (cmd == "ls") {
            cmdLs();
        } else if (cmd == "cd") {
            std::string arg;
            if (iss >> arg) cmdCd(arg);
            else std::cout << "cd: missing argument\n";
        } else if (cmd == "cat") {
            std::string arg;
            if (iss >> arg) cmdCat(arg);
            else std::cout << "cat: missing argument\n";
        } else if (cmd == "touch") {
            std::string arg;
            if (iss >> arg) cmdTouch(arg);
            else std::cout << "touch: missing argument\n";
        } else if (cmd == "mkdir") {
            std::string arg;
            if (iss >> arg) cmdMkdir(arg);
            else std::cout << "mkdir: missing argument\n";
        } else if (cmd == "rm") {
            std::string arg;
            if (iss >> arg) cmdRm(arg);
            else std::cout << "rm: missing argument\n";
        } else if (cmd == "restore") {
            std::string arg;
            if (iss >> arg) cmdRestore(arg);
            else std::cout << "restore: missing argument\n";
        } else if (cmd == "mv") {
            std::string old, newName;
            if (iss >> old >> newName) cmdMv(old, newName);
            else std::cout << "mv: requires two arguments\n";
        } else if (cmd == "edit") {
            std::string arg;
            if (iss >> arg) cmdEdit(arg);
            else std::cout << "edit: missing argument\n";
        } else {
            std::cout << "Unknown command: " << cmd << "\n";
        }
    }
}
