
#ifndef DIRECTORY_H
#define DIRECTORY_H

#include "Entry.h"
#include <vector>

class Directory : public Entry {
private:
    std::vector<Entry*> children;

public:
    explicit Directory(const std::string& name, Directory* parent = nullptr);
    ~Directory();

    std::string displayName() const override;
    std::vector<Entry*> listAll() const override;     // возвращает копию вектора
    Directory* asDirectory() override;
    void edit() override;
    void hide() override;
    void restore() override;
    void display() const override;

    void addEntry(Entry* entry);
    void replaceEntry(Entry* oldEntry, Entry* newEntry);
    Entry* find(const std::string& name) const;
};

#endif
