#ifndef ENTRY_H
#define ENTRY_H

#include <string>
#include <vector>

class Directory; // forward declaration

class Entry {
protected:
    std::string name;
    Directory* parent;

public:
    Entry(const std::string& name, Directory* parent = nullptr);
    virtual ~Entry() = default;

    // Отображаемое имя (у директорий добавляется '/')
    virtual std::string displayName() const = 0;

    // Перечисление содержимого (актуально только для Directory)
    virtual std::vector<Entry*> listAll() const;

    // Приведение к Directory (нарушает инкапсуляцию, принято осознанно)
    virtual Directory* asDirectory();

    // Редактирование содержимого
    virtual void edit();

    // Мягкое удаление – добавить точку в начало имени
    virtual void hide();

    // Восстановление – убрать начальную точку
    virtual void restore();

    // Скрыт ли элемент (имя начинается с '.')
    bool isHidden() const;

    // Вывод содержимого (для файлов – контент, для директории – сообщение)
    virtual void display() const = 0;

    // Геттер/сеттер имени (для mv)
    const std::string& getName() const;
    void setName(const std::string& newName);

    Directory* getParent() const;
    void setParent(Directory* p);
};

#endif
