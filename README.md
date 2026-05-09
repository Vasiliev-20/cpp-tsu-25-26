# OOP Terminal (C++)

Учебный проект объектно-ориентированного терминала, демонстрирующий:

- иерархию классов (`Entry`, `Directory`, `SingleLineFile`, `MultiLineFile`, `PostponedFile`)
- полиморфизм (методы `edit`, `display`, `hide`, `restore`)
- CRUD-операции над файлами и директориями
- мягкое удаление (скрытие через точку в имени)
- разработку «сверху вниз» от интерфейса терминала

## Сборка и запуск

Компилятор с поддержкой C++11 или новее.

### Вручную (g++)
```bash
g++ -std=c++11 -o terminal main.cpp Terminal.cpp Directory.cpp Entry.cpp FileClasses.cpp
./terminal
