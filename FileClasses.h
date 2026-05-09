FileClasses.h
#ifndef FILECLASSES_H
#define FILECLASSES_H

#include "Entry.h"
#include <vector>
#include <string>

// Однострочный файл
class SingleLineFile : public Entry {
private:
    std::string content;

public:
    SingleLineFile(const std::string& name, Directory* parent = nullptr);
    std::string displayName() const override;
    void edit() override;
    void display() const override;
};

// Многострочный файл
class MultiLineFile : public Entry {
private:
    std::vector<std::string> lines;

public:
    MultiLineFile(const std::string& name, Directory* parent = nullptr);
    std::string displayName() const override;
    void edit() override;
    void display() const override;
};

// Отложенный файл (зигота)
class PostponedFile : public Entry {
public:
    PostponedFile(const std::string& name, Directory* parent = nullptr);
    std::string displayName() const override;
    void edit() override;
    void display() const override;
};

#endif
