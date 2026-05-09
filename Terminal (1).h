Terminal.h
#ifndef TERMINAL_H
#define TERMINAL_H

#include <vector>
#include <string>

class Directory;

class Terminal {
private:
    Directory* root;
    std::vector<Directory*> path; // текущий путь от root до current

    std::string prompt() const;
    void cmdLs();
    void cmdCd(const std::string& arg);
    void cmdCat(const std::string& arg);
    void cmdTouch(const std::string& arg);
    void cmdMkdir(const std::string& arg);
    void cmdRm(const std::string& arg);
    void cmdRestore(const std::string& arg);
    void cmdMv(const std::string& oldName, const std::string& newName);
    void cmdEdit(const std::string& arg);
    void cmdHelp() const;

public:
    Terminal();
    ~Terminal();
    void run();
};

#endif
