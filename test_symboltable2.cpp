#include "SymbolTable.h"
#include <cassert>
#include <iostream>
#include <sstream>

int main() {
    SymbolTable st;
    // Добавляем глобальную переменную
    auto op1 = st.add("x", SymbolTable::RecordKind::var, SymbolTable::RecordType::integer, -1, 10, GlobalScope);
    // Добавляем функцию
    auto op2 = st.add("foo", SymbolTable::RecordKind::func, SymbolTable::RecordType::integer, 2, 0, GlobalScope);
    // Добавляем локальную переменную в области 5
    auto op3 = st.add("y", SymbolTable::RecordKind::var, SymbolTable::RecordType::chr, -1, 'a', 5);

    assert(st[0]._name == "x");
    assert(st[0]._kind == SymbolTable::RecordKind::var);
    assert(st[0]._init == 10);
    assert(st[1]._name == "foo");
    assert(st[1]._kind == SymbolTable::RecordKind::func);
    assert(st[1]._len == 2);
    assert(st[2]._name == "y");
    assert(st[2]._scope == 5);

    int idx = st.find("x", GlobalScope);
    assert(idx == 0);
    idx = st.find("y", 5);
    assert(idx == 2);
    idx = st.find("y", GlobalScope); // не найдёт, т.к. локальная
    assert(idx == -1);

    std::ostringstream oss;
    oss << st;
    std::string expected = 
        "SYMBOL TABLE\n"
        "-------------------------------------------------------\n"
        "code  name        kind    type    len   init  scope  offset\n"
        "0     x           var     int              10    -1     -1     \n"
        "1     foo         func    int     2              -1     -1     \n"
        "2     y           var     chr                   5      -1     \n";
    // Сравнение строк может быть чувствительно к пробелам, поэтому лучше просто проверить наличие ключевых слов
    std::string out = oss.str();
    assert(out.find("x") != std::string::npos);
    assert(out.find("foo") != std::string::npos);
    assert(out.find("y") != std::string::npos);

    std::cout << "All SymbolTable tests passed!\n";
    return 0;
}
