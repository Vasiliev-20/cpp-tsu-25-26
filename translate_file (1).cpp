#include "Translator.h"
#include <fstream>
#include <iostream>

int main() {
    std::ifstream file("expr.minic");
    if (!file) {
        std::cerr << "Cannot open file\n";
        return 1;
    }
    Translator tr(file);
    tr.translate();
    return 0;
}
