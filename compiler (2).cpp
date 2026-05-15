#include "Scaner.h"
#include <fstream>
#include <iostream>

using namespace std;

int main() {
    ifstream ifile("_num_.minic");
    Scanner scanner(ifile);
    for (;;) {
        Token currentLexem = scanner.getNextToken();
        // вывод в формате [тип, значение]
        currentLexem.print(cout);
        cout << endl;
        // если это ошибка или конец файла - завершаем цикл
        if (currentLexem.type() == LexemType::error || currentLexem.type() == LexemType::eof) {
            break;
        }
    }
    return 0;
}
